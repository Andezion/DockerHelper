#include "docker_manager.h"
#include <wx/thread.h>

wxDEFINE_EVENT(wxEVT_UPDATE_COMPLETE, wxThreadEvent);

struct UpdateData {
    std::vector<ContainerInfo> runningContainers;
    std::vector<ContainerInfo> stoppedContainers;
    std::vector<ImageInfo> unusedImages;
    std::vector<VolumeInfo> unusedVolumes;
    SystemInfo systemInfo;
};

class UpdateThread : public wxThread {
public:
    UpdateThread(DockerManagerFrame* handler) 
        : wxThread(wxTHREAD_DETACHED), m_handler(handler) {}
    
protected:
    virtual ExitCode Entry() override {
        UpdateData* data = new UpdateData();
        
        data->runningContainers = DockerCommands::GetRunningContainers();
        data->stoppedContainers = DockerCommands::GetStoppedContainers();
        data->unusedImages = DockerCommands::GetUnusedImages();
        data->unusedVolumes = DockerCommands::GetUnusedVolumes();
        data->systemInfo = DockerCommands::GetSystemInfo();
        
        wxThreadEvent* event = new wxThreadEvent(wxEVT_UPDATE_COMPLETE);
        event->SetPayload(data);
        wxQueueEvent(m_handler, event);
        
        return (ExitCode)0;
    }
    
private:
    DockerManagerFrame* m_handler;
};

wxBEGIN_EVENT_TABLE(DockerManagerFrame, wxFrame)
    EVT_BUTTON(ID_STOP, DockerManagerFrame::OnStop)
    EVT_BUTTON(ID_STOP_ALL, DockerManagerFrame::OnStopAll)
    EVT_BUTTON(ID_REMOVE_CONTAINER, DockerManagerFrame::OnRemoveContainer)
    EVT_BUTTON(ID_REMOVE_IMAGE, DockerManagerFrame::OnRemoveImage)
    EVT_BUTTON(ID_REMOVE_VOLUME, DockerManagerFrame::OnRemoveVolume)
    EVT_BUTTON(ID_PRUNE_ALL, DockerManagerFrame::OnPruneAll)
    EVT_BUTTON(ID_REFRESH, DockerManagerFrame::OnRefresh)
    EVT_TIMER(ID_TIMER, DockerManagerFrame::OnTimer)
    EVT_CLOSE(DockerManagerFrame::OnClose)
    EVT_LIST_ITEM_SELECTED(ID_RUNNING_LIST, DockerManagerFrame::OnRunningItemSelected)
    EVT_LIST_ITEM_SELECTED(ID_STOPPED_LIST, DockerManagerFrame::OnStoppedItemSelected)
    EVT_LIST_ITEM_SELECTED(ID_IMAGES_LIST, DockerManagerFrame::OnImageItemSelected)
    EVT_LIST_ITEM_SELECTED(ID_VOLUMES_LIST, DockerManagerFrame::OnVolumeItemSelected)
    EVT_THREAD(ID_UPDATE_COMPLETE, DockerManagerFrame::OnUpdateComplete)
wxEND_EVENT_TABLE()

DockerManagerFrame::DockerManagerFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1000, 800)), 
      isUpdating(false) {
    
    wxPanel* mainPanel = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    CreateSystemInfoPanel(mainPanel, mainSizer);
    
    notebook = new wxNotebook(mainPanel, wxID_ANY);
    CreateRunningPanel();
    CreateCleanupPanel();
    
    notebook->AddPage(runningPanel, wxT("Active containers"), true);
    notebook->AddPage(cleanupPanel, wxT("Cleanup traces"));
    
    mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 5);
    
    refreshButton = new wxButton(mainPanel, ID_REFRESH, wxT("Refresh all"));
    mainSizer->Add(refreshButton, 0, wxALIGN_CENTER | wxALL, 5);
    
    mainPanel->SetSizer(mainSizer);
    
    refreshTimer = new wxTimer(this, ID_TIMER);
    refreshTimer->Start(3000);
    
    RefreshAllAsync();
    Centre();
}

DockerManagerFrame::~DockerManagerFrame() {
    if (refreshTimer) {
        refreshTimer->Stop();
        delete refreshTimer;
    }
}


void DockerManagerFrame::CreateSystemInfoPanel(wxPanel* parent, wxSizer* sizer) {
    wxStaticBoxSizer* infoBox = new wxStaticBoxSizer(wxHORIZONTAL, parent, 
                                                      wxT("System Information"));
    
    cpuLabel = new wxStaticText(parent, wxID_ANY, wxT("CPU: 0%"));
    memLabel = new wxStaticText(parent, wxID_ANY, wxT("Memory: 0"));
    containersLabel = new wxStaticText(parent, wxID_ANY, wxT("Containers: 0"));
    
    wxFont boldFont = cpuLabel->GetFont();
    boldFont.SetWeight(wxFONTWEIGHT_BOLD);
    cpuLabel->SetFont(boldFont);
    memLabel->SetFont(boldFont);
    containersLabel->SetFont(boldFont);
    
    infoBox->Add(cpuLabel, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    infoBox->Add(memLabel, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    infoBox->Add(containersLabel, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    
    sizer->Add(infoBox, 0, wxEXPAND | wxALL, 5);
}

void DockerManagerFrame::CreateRunningPanel() {
    runningPanel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    runningList = new wxListCtrl(runningPanel, ID_RUNNING_LIST, 
                                  wxDefaultPosition, wxDefaultSize, 
                                  wxLC_REPORT | wxLC_SINGLE_SEL);
    runningList->AppendColumn(wxT("ID"), wxLIST_FORMAT_LEFT, 120);
    runningList->AppendColumn(wxT("Name"), wxLIST_FORMAT_LEFT, 200);
    runningList->AppendColumn(wxT("Status"), wxLIST_FORMAT_LEFT, 250);
    runningList->AppendColumn(wxT("Image"), wxLIST_FORMAT_LEFT, 300);
    
    sizer->Add(runningList, 1, wxEXPAND | wxALL, 5);
    
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    
    stopButton = new wxButton(runningPanel, ID_STOP, wxT("Stop container"));
    stopButton->Enable(false);
    buttonSizer->Add(stopButton, 0, wxALL, 5);
    
    stopAllButton = new wxButton(runningPanel, ID_STOP_ALL, wxT("Stop ALL"));
    stopAllButton->SetBackgroundColour(wxColour(255, 165, 0));
    buttonSizer->Add(stopAllButton, 0, wxALL, 5);
    
    sizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);
    runningPanel->SetSizer(sizer);
}

void DockerManagerFrame::CreateCleanupPanel() {
    cleanupPanel = new wxPanel(notebook);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    wxStaticBoxSizer* stoppedBox = new wxStaticBoxSizer(wxVERTICAL, cleanupPanel, 
                                                         wxT("Stopped containers"));
    stoppedList = new wxListCtrl(cleanupPanel, ID_STOPPED_LIST, 
                                  wxDefaultPosition, wxDefaultSize,
                                  wxLC_REPORT | wxLC_SINGLE_SEL);
    stoppedList->AppendColumn(wxT("ID"), wxLIST_FORMAT_LEFT, 120);
    stoppedList->AppendColumn(wxT("Name"), wxLIST_FORMAT_LEFT, 200);
    stoppedList->AppendColumn(wxT("Status"), wxLIST_FORMAT_LEFT, 200);
    stoppedList->AppendColumn(wxT("Image"), wxLIST_FORMAT_LEFT, 250);
    
    stoppedBox->Add(stoppedList, 1, wxEXPAND | wxALL, 5);
    
    removeContainerButton = new wxButton(cleanupPanel, ID_REMOVE_CONTAINER, 
                                          wxT("Remove container"));
    removeContainerButton->Enable(false);
    stoppedBox->Add(removeContainerButton, 0, wxALIGN_CENTER | wxALL, 5);
    mainSizer->Add(stoppedBox, 3, wxEXPAND | wxALL, 5);
    
    wxStaticBoxSizer* imagesBox = new wxStaticBoxSizer(wxVERTICAL, cleanupPanel,
                                                        wxT("Unused images"));
    imagesList = new wxListCtrl(cleanupPanel, ID_IMAGES_LIST, 
                                 wxDefaultPosition, wxSize(-1, 120),
                                 wxLC_REPORT | wxLC_SINGLE_SEL);
    imagesList->AppendColumn(wxT("ID"), wxLIST_FORMAT_LEFT, 120);
    imagesList->AppendColumn(wxT("Repository"), wxLIST_FORMAT_LEFT, 250);
    imagesList->AppendColumn(wxT("Tag"), wxLIST_FORMAT_LEFT, 100);
    imagesList->AppendColumn(wxT("Size"), wxLIST_FORMAT_LEFT, 100);
    
    imagesBox->Add(imagesList, 0, wxEXPAND | wxALL, 5);
    
    removeImageButton = new wxButton(cleanupPanel, ID_REMOVE_IMAGE, 
                                      wxT("Remove image"));
    removeImageButton->Enable(false);
    imagesBox->Add(removeImageButton, 0, wxALIGN_CENTER | wxALL, 5);
    mainSizer->Add(imagesBox, 0, wxEXPAND | wxALL, 5);
    
    wxStaticBoxSizer* volumesBox = new wxStaticBoxSizer(wxVERTICAL, cleanupPanel,
                                                         wxT("Unused volumes"));
    volumesList = new wxListCtrl(cleanupPanel, ID_VOLUMES_LIST, 
                                  wxDefaultPosition, wxSize(-1, 80),
                                  wxLC_REPORT | wxLC_SINGLE_SEL);
    volumesList->AppendColumn(wxT("Name"), wxLIST_FORMAT_LEFT, 400);
    volumesList->AppendColumn(wxT("Driver"), wxLIST_FORMAT_LEFT, 150);
    
    volumesBox->Add(volumesList, 0, wxEXPAND | wxALL, 5);
    
    removeVolumeButton = new wxButton(cleanupPanel, ID_REMOVE_VOLUME, 
                                       wxT("Remove volume"));
    removeVolumeButton->Enable(false);
    volumesBox->Add(removeVolumeButton, 0, wxALIGN_CENTER | wxALL, 5);
    mainSizer->Add(volumesBox, 0, wxEXPAND | wxALL, 5);
    
    pruneAllButton = new wxButton(cleanupPanel, ID_PRUNE_ALL, 
                                   wxT("PRUNE ALL (Docker Prune)"));
    pruneAllButton->SetBackgroundColour(*wxRED);
    pruneAllButton->SetForegroundColour(*wxWHITE);
    mainSizer->Add(pruneAllButton, 0, wxALIGN_CENTER | wxALL, 5);
    
    cleanupPanel->SetSizer(mainSizer);
}

void DockerManagerFrame::RefreshAllAsync() {
    if (isUpdating) {
        return;
    }
    
    isUpdating = true;
    UpdateThread* updateThread = new UpdateThread(this);
    
    if (updateThread->Run() != wxTHREAD_NO_ERROR) {
        delete updateThread;
        isUpdating = false;
    }
}

void DockerManagerFrame::OnUpdateComplete(wxThreadEvent& event) {
    UpdateData* data = event.GetPayload<UpdateData*>();
    
    if (data) {
        PopulateRunningContainers(data->runningContainers);
        PopulateStoppedContainers(data->stoppedContainers);
        PopulateUnusedImages(data->unusedImages);
        PopulateUnusedVolumes(data->unusedVolumes);
        UpdateSystemInfoUI(data->systemInfo);
        
        delete data;
    }
    
    isUpdating = false;
}

void DockerManagerFrame::PopulateRunningContainers(
    const std::vector<ContainerInfo>& containers) {
    runningList->DeleteAllItems();
    
    int row = 0;
    for (const auto& container : containers) {
        long index = runningList->InsertItem(row, 
                                              wxString::FromUTF8(container.id.c_str()));
        runningList->SetItem(index, 1, wxString::FromUTF8(container.name.c_str()));
        runningList->SetItem(index, 2, wxString::FromUTF8(container.status.c_str()));
        runningList->SetItem(index, 3, wxString::FromUTF8(container.image.c_str()));
        row++;
    }
}

void DockerManagerFrame::PopulateStoppedContainers(
    const std::vector<ContainerInfo>& containers) {
    stoppedList->DeleteAllItems();
    
    int row = 0;
    for (const auto& container : containers) {
        long index = stoppedList->InsertItem(row, 
                                              wxString::FromUTF8(container.id.c_str()));
        stoppedList->SetItem(index, 1, wxString::FromUTF8(container.name.c_str()));
        stoppedList->SetItem(index, 2, wxString::FromUTF8(container.status.c_str()));
        stoppedList->SetItem(index, 3, wxString::FromUTF8(container.image.c_str()));
        row++;
    }
}

void DockerManagerFrame::PopulateUnusedImages(
    const std::vector<ImageInfo>& images) {
    imagesList->DeleteAllItems();
    
    int row = 0;
    for (const auto& image : images) {
        long index = imagesList->InsertItem(row, 
                                             wxString::FromUTF8(image.id.c_str()));
        imagesList->SetItem(index, 1, wxString::FromUTF8(image.repository.c_str()));
        imagesList->SetItem(index, 2, wxString::FromUTF8(image.tag.c_str()));
        imagesList->SetItem(index, 3, wxString::FromUTF8(image.size.c_str()));
        row++;
    }
}

void DockerManagerFrame::PopulateUnusedVolumes(
    const std::vector<VolumeInfo>& volumes) {
    volumesList->DeleteAllItems();
    
    int row = 0;
    for (const auto& volume : volumes) {
        long index = volumesList->InsertItem(row, 
                                              wxString::FromUTF8(volume.name.c_str()));
        volumesList->SetItem(index, 1, wxString::FromUTF8(volume.driver.c_str()));
        row++;
    }
}

void DockerManagerFrame::UpdateSystemInfoUI(const SystemInfo& info) {
    cpuLabel->SetLabel(wxString::Format(wxT("CPU: %.1f%%"), info.cpu_usage));
    memLabel->SetLabel(wxString::Format(wxT("Memory: %s"), 
                       wxString::FromUTF8(info.mem_usage.c_str())));
    containersLabel->SetLabel(wxString::Format(wxT("Containers: %d"), 
                              info.container_count));
}

void DockerManagerFrame::OnStop(wxCommandEvent& event) {
    long selected = runningList->GetNextItem(-1, wxLIST_NEXT_ALL, 
                                              wxLIST_STATE_SELECTED);
    if (selected == -1) return;
    
    wxString id = runningList->GetItemText(selected, 0);
    wxString name = runningList->GetItemText(selected, 1);
    
    int response = wxMessageBox(
        wxString::Format(wxT("Stop container '%s' (%s)?"), name, id),
        wxT("Confirmation"),
        wxYES_NO | wxICON_QUESTION,
        this
    );
    
    if (response == wxYES) {
        DockerCommands::StopContainer(std::string(id.mb_str()));
        wxSleep(1);
        RefreshAllAsync();
        wxMessageBox(wxT("Container stopped"), wxT("Success"), 
                     wxOK | wxICON_INFORMATION);
    }
}

void DockerManagerFrame::OnStopAll(wxCommandEvent& event) {
    int response = wxMessageBox(
        wxT("WARNING! Stop ALL running containers?\n\n"
            "This action will affect all active containers!"),
        wxT("Confirmation"),
        wxYES_NO | wxICON_WARNING,
        this
    );
    
    if (response == wxYES) {
        DockerCommands::StopAllContainers();
        wxSleep(2);
        RefreshAllAsync();
        wxMessageBox(wxT("All containers stopped"), wxT("Success"), 
                     wxOK | wxICON_INFORMATION);
    }
}

void DockerManagerFrame::OnRemoveContainer(wxCommandEvent& event) {
    long selected = stoppedList->GetNextItem(-1, wxLIST_NEXT_ALL, 
                                              wxLIST_STATE_SELECTED);
    if (selected == -1) return;
    
    wxString id = stoppedList->GetItemText(selected, 0);
    wxString name = stoppedList->GetItemText(selected, 1);
    
    int response = wxMessageBox(
        wxString::Format(wxT("Remove container '%s' (%s)?"), name, id),
        wxT("Confirmation"),
        wxYES_NO | wxICON_WARNING,
        this
    );
    
    if (response == wxYES) {
        DockerCommands::RemoveContainer(std::string(id.mb_str()));
        RefreshAllAsync();
        wxMessageBox(wxT("Container removed"), wxT("Success"), 
                     wxOK | wxICON_INFORMATION);
    }
}

void DockerManagerFrame::OnRemoveImage(wxCommandEvent& event) {
    long selected = imagesList->GetNextItem(-1, wxLIST_NEXT_ALL, 
                                             wxLIST_STATE_SELECTED);
    if (selected == -1) return;
    
    wxString id = imagesList->GetItemText(selected, 0);
    
    int response = wxMessageBox(
        wxString::Format(wxT("Remove image %s?"), id),
        wxT("Confirmation"),
        wxYES_NO | wxICON_WARNING,
        this
    );
    
    if (response == wxYES) {
        DockerCommands::RemoveImage(std::string(id.mb_str()));
        RefreshAllAsync();
        wxMessageBox(wxT("Image removed"), wxT("Success"), 
                     wxOK | wxICON_INFORMATION);
    }
}

void DockerManagerFrame::OnRemoveVolume(wxCommandEvent& event) {
    long selected = volumesList->GetNextItem(-1, wxLIST_NEXT_ALL, 
                                              wxLIST_STATE_SELECTED);
    if (selected == -1) return;
    
    wxString name = volumesList->GetItemText(selected, 0);
    
    int response = wxMessageBox(
        wxString::Format(wxT("Remove volume %s?"), name),
        wxT("Confirmation"),
        wxYES_NO | wxICON_WARNING,
        this
    );
    
    if (response == wxYES) {
        DockerCommands::RemoveVolume(std::string(name.mb_str()));
        RefreshAllAsync();
        wxMessageBox(wxT("Volume removed"), wxT("Success"), 
                     wxOK | wxICON_INFORMATION);
    }
}

void DockerManagerFrame::OnPruneAll(wxCommandEvent& event) {
    int response = wxMessageBox(
        wxT("WARNING! This will remove:\n"
            "- All stopped containers\n"
            "- All unused images\n"
            "- All unused volumes\n"
            "- All build cache\n\n"
            "Continue?"),
        wxT("Confirmation"),
        wxYES_NO | wxICON_WARNING,
        this
    );
    
    if (response == wxYES) {
        wxMessageBox(wxT("Pruning started, this may take some time..."), 
                     wxT("Information"), wxOK | wxICON_INFORMATION);
        DockerCommands::PruneAll();
        wxSleep(2);
        RefreshAllAsync();
        wxMessageBox(wxT("Pruning completed!"), wxT("Success"), 
                     wxOK | wxICON_INFORMATION);
    }
}

void DockerManagerFrame::OnRefresh(wxCommandEvent& event) {
    RefreshAllAsync();
}

void DockerManagerFrame::OnTimer(wxTimerEvent& event) {
    RefreshAllAsync();
}

void DockerManagerFrame::OnClose(wxCloseEvent& event) {
    if (refreshTimer) {
        refreshTimer->Stop();
    }
    Destroy();
}

void DockerManagerFrame::OnRunningItemSelected(wxListEvent& event) {
    stopButton->Enable(true);
}

void DockerManagerFrame::OnStoppedItemSelected(wxListEvent& event) {
    removeContainerButton->Enable(true);
}

void DockerManagerFrame::OnImageItemSelected(wxListEvent& event) {
    removeImageButton->Enable(true);
}

void DockerManagerFrame::OnVolumeItemSelected(wxListEvent& event) {
    removeVolumeButton->Enable(true);
}

wxIMPLEMENT_APP(DockerManagerApp);

bool DockerManagerApp::OnInit() {
    DockerManagerFrame* frame = new DockerManagerFrame(wxT("Docker Manager"));
    frame->Show(true);
    return true;
}
