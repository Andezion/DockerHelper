#include "docker_manager.h"
#include <cstdio>
#include <array>
#include <memory>
#include <sstream>

wxBEGIN_EVENT_TABLE(DockerManagerFrame, wxFrame)
    EVT_BUTTON(ID_STOP, DockerManagerFrame::OnStop)
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
wxEND_EVENT_TABLE()

DockerManagerFrame::DockerManagerFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1000, 700)) {
    
    wxPanel* mainPanel = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    CreateSystemInfoPanel(mainPanel, mainSizer);
    
    notebook = new wxNotebook(mainPanel, wxID_ANY);
    
    CreateRunningPanel();
    CreateCleanupPanel();
    
    notebook->AddPage(runningPanel, wxT("Активные контейнеры"), true);
    notebook->AddPage(cleanupPanel, wxT("Очистка следов"));
    
    mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 5);
    
    refreshButton = new wxButton(mainPanel, ID_REFRESH, wxT("Обновить всё"));
    mainSizer->Add(refreshButton, 0, wxALIGN_CENTER | wxALL, 5);
    
    mainPanel->SetSizer(mainSizer);
    
    refreshTimer = new wxTimer(this, ID_TIMER);
    refreshTimer->Start(5000);
    
    RefreshAll();
    
    Centre();
}

void DockerManagerFrame::CreateSystemInfoPanel(wxPanel* parent, wxSizer* sizer) {
    wxStaticBoxSizer* infoBox = new wxStaticBoxSizer(wxHORIZONTAL, parent, wxT("Системная информация"));
    
    cpuLabel = new wxStaticText(parent, wxID_ANY, wxT("CPU: 0%"));
    memLabel = new wxStaticText(parent, wxID_ANY, wxT("Память: 0"));
    containersLabel = new wxStaticText(parent, wxID_ANY, wxT("Контейнеров: 0"));
    
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
    
    runningList = new wxListCtrl(runningPanel, ID_RUNNING_LIST, wxDefaultPosition, 
                                  wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    runningList->AppendColumn(wxT("ID"), wxLIST_FORMAT_LEFT, 120);
    runningList->AppendColumn(wxT("Имя"), wxLIST_FORMAT_LEFT, 200);
    runningList->AppendColumn(wxT("Статус"), wxLIST_FORMAT_LEFT, 200);
    runningList->AppendColumn(wxT("Образ"), wxLIST_FORMAT_LEFT, 250);
    
    sizer->Add(runningList, 1, wxEXPAND | wxALL, 5);
    
    stopButton = new wxButton(runningPanel, ID_STOP, wxT("Остановить контейнер"));
    stopButton->Enable(false);
    sizer->Add(stopButton, 0, wxALIGN_CENTER | wxALL, 5);
    
    runningPanel->SetSizer(sizer);
}

void DockerManagerFrame::CreateCleanupPanel() {
    cleanupPanel = new wxPanel(notebook);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    wxStaticBoxSizer* stoppedBox = new wxStaticBoxSizer(wxVERTICAL, cleanupPanel, 
                                                         wxT("Остановленные контейнеры"));
    stoppedList = new wxListCtrl(cleanupPanel, ID_STOPPED_LIST, wxDefaultPosition,
                                  wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    stoppedList->AppendColumn(wxT("ID"), wxLIST_FORMAT_LEFT, 120);
    stoppedList->AppendColumn(wxT("Имя"), wxLIST_FORMAT_LEFT, 200);
    stoppedList->AppendColumn(wxT("Статус"), wxLIST_FORMAT_LEFT, 200);
    stoppedList->AppendColumn(wxT("Образ"), wxLIST_FORMAT_LEFT, 250);
    
    stoppedBox->Add(stoppedList, 1, wxEXPAND | wxALL, 5);
    removeContainerButton = new wxButton(cleanupPanel, ID_REMOVE_CONTAINER, 
                                          wxT("Удалить контейнер"));
    removeContainerButton->Enable(false);
    stoppedBox->Add(removeContainerButton, 0, wxALIGN_CENTER | wxALL, 5);
    mainSizer->Add(stoppedBox, 1, wxEXPAND | wxALL, 5);
    
    wxStaticBoxSizer* imagesBox = new wxStaticBoxSizer(wxVERTICAL, cleanupPanel,
                                                        wxT("Неиспользуемые образы"));
    imagesList = new wxListCtrl(cleanupPanel, ID_IMAGES_LIST, wxDefaultPosition,
                                 wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    imagesList->AppendColumn(wxT("ID"), wxLIST_FORMAT_LEFT, 120);
    imagesList->AppendColumn(wxT("Репозиторий"), wxLIST_FORMAT_LEFT, 250);
    imagesList->AppendColumn(wxT("Тег"), wxLIST_FORMAT_LEFT, 100);
    imagesList->AppendColumn(wxT("Размер"), wxLIST_FORMAT_LEFT, 100);
    
    imagesBox->Add(imagesList, 1, wxEXPAND | wxALL, 5);
    removeImageButton = new wxButton(cleanupPanel, ID_REMOVE_IMAGE, wxT("Удалить образ"));
    removeImageButton->Enable(false);
    imagesBox->Add(removeImageButton, 0, wxALIGN_CENTER | wxALL, 5);
    mainSizer->Add(imagesBox, 1, wxEXPAND | wxALL, 5);
    
    wxStaticBoxSizer* volumesBox = new wxStaticBoxSizer(wxVERTICAL, cleanupPanel,
                                                         wxT("Неиспользуемые volumes"));
    volumesList = new wxListCtrl(cleanupPanel, ID_VOLUMES_LIST, wxDefaultPosition,
                                  wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    volumesList->AppendColumn(wxT("Имя"), wxLIST_FORMAT_LEFT, 400);
    volumesList->AppendColumn(wxT("Драйвер"), wxLIST_FORMAT_LEFT, 150);
    
    volumesBox->Add(volumesList, 1, wxEXPAND | wxALL, 5);
    removeVolumeButton = new wxButton(cleanupPanel, ID_REMOVE_VOLUME, wxT("Удалить volume"));
    removeVolumeButton->Enable(false);
    volumesBox->Add(removeVolumeButton, 0, wxALIGN_CENTER | wxALL, 5);
    mainSizer->Add(volumesBox, 1, wxEXPAND | wxALL, 5);
    
    pruneAllButton = new wxButton(cleanupPanel, ID_PRUNE_ALL, 
                                   wxT("ОЧИСТИТЬ ВСЁ (Docker Prune)"));
    pruneAllButton->SetBackgroundColour(*wxRED);
    pruneAllButton->SetForegroundColour(*wxWHITE);
    mainSizer->Add(pruneAllButton, 0, wxALIGN_CENTER | wxALL, 5);
    
    cleanupPanel->SetSizer(mainSizer);
}

std::string DockerManagerFrame::ExecuteCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    
    if (!pipe) {
        return "";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    return result;
}

void DockerManagerFrame::UpdateRunningContainers() {
    runningList->DeleteAllItems();
    
    std::string scriptPath = "../scripts/docker_info.sh";
    std::string output = ExecuteCommand(scriptPath + " running");
    
    std::istringstream stream(output);
    std::string line;
    int row = 0;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        std::istringstream lineStream(line);
        std::string id, name, status, image;
        
        std::getline(lineStream, id, '|');
        std::getline(lineStream, name, '|');
        std::getline(lineStream, status, '|');
        std::getline(lineStream, image, '|');
        
        long index = runningList->InsertItem(row, wxString::FromUTF8(id.c_str()));
        runningList->SetItem(index, 1, wxString::FromUTF8(name.c_str()));
        runningList->SetItem(index, 2, wxString::FromUTF8(status.c_str()));
        runningList->SetItem(index, 3, wxString::FromUTF8(image.c_str()));
        row++;
    }
}

void DockerManagerFrame::UpdateStoppedContainers() {
    stoppedList->DeleteAllItems();
    
    std::string scriptPath = "../scripts/docker_info.sh";
    std::string output = ExecuteCommand(scriptPath + " stopped");
    
    std::istringstream stream(output);
    std::string line;
    int row = 0;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        std::istringstream lineStream(line);
        std::string id, name, status, image;
        
        std::getline(lineStream, id, '|');
        std::getline(lineStream, name, '|');
        std::getline(lineStream, status, '|');
        std::getline(lineStream, image, '|');
        
        long index = stoppedList->InsertItem(row, wxString::FromUTF8(id.c_str()));
        stoppedList->SetItem(index, 1, wxString::FromUTF8(name.c_str()));
        stoppedList->SetItem(index, 2, wxString::FromUTF8(status.c_str()));
        stoppedList->SetItem(index, 3, wxString::FromUTF8(image.c_str()));
        row++;
    }
}

void DockerManagerFrame::UpdateUnusedImages() {
    imagesList->DeleteAllItems();
    
    std::string scriptPath = "../scripts/docker_info.sh";
    std::string output = ExecuteCommand(scriptPath + " unused_images");
    
    std::istringstream stream(output);
    std::string line;
    int row = 0;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        std::istringstream lineStream(line);
        std::string id, repo, tag, size;
        
        std::getline(lineStream, id, '|');
        std::getline(lineStream, repo, '|');
        std::getline(lineStream, tag, '|');
        std::getline(lineStream, size, '|');
        
        long index = imagesList->InsertItem(row, wxString::FromUTF8(id.c_str()));
        imagesList->SetItem(index, 1, wxString::FromUTF8(repo.c_str()));
        imagesList->SetItem(index, 2, wxString::FromUTF8(tag.c_str()));
        imagesList->SetItem(index, 3, wxString::FromUTF8(size.c_str()));
        row++;
    }
}

void DockerManagerFrame::UpdateUnusedVolumes() {
    volumesList->DeleteAllItems();
    
    std::string scriptPath = "../scripts/docker_info.sh";
    std::string output = ExecuteCommand(scriptPath + " unused_volumes");
    
    std::istringstream stream(output);
    std::string line;
    int row = 0;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        std::istringstream lineStream(line);
        std::string name, driver;
        
        std::getline(lineStream, name, '|');
        std::getline(lineStream, driver, '|');
        
        long index = volumesList->InsertItem(row, wxString::FromUTF8(name.c_str()));
        volumesList->SetItem(index, 1, wxString::FromUTF8(driver.c_str()));
        row++;
    }
}

void DockerManagerFrame::UpdateSystemInfo() {
    std::string scriptPath = "../scripts/docker_info.sh";
    std::string output = ExecuteCommand(scriptPath + " sysinfo");
    
    if (output.empty()) {
        cpuLabel->SetLabel(wxT("CPU: 0%"));
        memLabel->SetLabel(wxT("Память: 0 MiB"));
        containersLabel->SetLabel(wxT("Контейнеров: 0"));
        return;
    }
    
    std::istringstream stream(output);
    std::string token;
    
    while (std::getline(stream, token, '|')) {
        size_t pos = token.find(':');
        if (pos != std::string::npos) {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            
            if (key == "CPU") {
                cpuLabel->SetLabel(wxString::Format(wxT("CPU: %s"), 
                                   wxString::FromUTF8(value.c_str())));
            } else if (key == "MEM") {
                memLabel->SetLabel(wxString::Format(wxT("Память: %s"), 
                                   wxString::FromUTF8(value.c_str())));
            } else if (key == "CONTAINERS") {
                containersLabel->SetLabel(wxString::Format(wxT("Контейнеров: %s"), 
                                          wxString::FromUTF8(value.c_str())));
            }
        }
    }
}

void DockerManagerFrame::RefreshAll() {
    UpdateRunningContainers();
    UpdateStoppedContainers();
    UpdateUnusedImages();
    UpdateUnusedVolumes();
    UpdateSystemInfo();
}

void DockerManagerFrame::OnStop(wxCommandEvent& event) {
    long selected = runningList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected == -1) return;
    
    wxString id = runningList->GetItemText(selected, 0);
    wxString name = runningList->GetItemText(selected, 1);
    
    int response = wxMessageBox(
        wxString::Format(wxT("Остановить контейнер '%s' (%s)?"), name, id),
        wxT("Подтверждение"),
        wxYES_NO | wxICON_QUESTION,
        this
    );
    
    if (response == wxYES) {
        std::string command = "../scripts/docker_info.sh stop " + std::string(id.mb_str());
        ExecuteCommand(command);
        RefreshAll();
        wxMessageBox(wxT("Контейнер остановлен"), wxT("Успех"), wxOK | wxICON_INFORMATION);
    }
}

void DockerManagerFrame::OnRemoveContainer(wxCommandEvent& event) {
    long selected = stoppedList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected == -1) return;
    
    wxString id = stoppedList->GetItemText(selected, 0);
    wxString name = stoppedList->GetItemText(selected, 1);
    
    int response = wxMessageBox(
        wxString::Format(wxT("Удалить контейнер '%s' (%s)?"), name, id),
        wxT("Подтверждение"),
        wxYES_NO | wxICON_WARNING,
        this
    );
    
    if (response == wxYES) {
        std::string command = "../scripts/docker_info.sh remove_container " + std::string(id.mb_str());
        ExecuteCommand(command);
        RefreshAll();
        wxMessageBox(wxT("Контейнер удалён"), wxT("Успех"), wxOK | wxICON_INFORMATION);
    }
}

void DockerManagerFrame::OnRemoveImage(wxCommandEvent& event) {
    long selected = imagesList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected == -1) return;
    
    wxString id = imagesList->GetItemText(selected, 0);
    
    int response = wxMessageBox(
        wxString::Format(wxT("Удалить образ %s?"), id),
        wxT("Подтверждение"),
        wxYES_NO | wxICON_WARNING,
        this
    );
    
    if (response == wxYES) {
        std::string command = "../scripts/docker_info.sh remove_image " + std::string(id.mb_str());
        ExecuteCommand(command);
        RefreshAll();
        wxMessageBox(wxT("Образ удалён"), wxT("Успех"), wxOK | wxICON_INFORMATION);
    }
}

void DockerManagerFrame::OnRemoveVolume(wxCommandEvent& event) {
    long selected = volumesList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected == -1) return;
    
    wxString name = volumesList->GetItemText(selected, 0);
    
    int response = wxMessageBox(
        wxString::Format(wxT("Удалить volume %s?"), name),
        wxT("Подтверждение"),
        wxYES_NO | wxICON_WARNING,
        this
    );
    
    if (response == wxYES) {
        std::string command = "../scripts/docker_info.sh remove_volume " + std::string(name.mb_str());
        ExecuteCommand(command);
        RefreshAll();
        wxMessageBox(wxT("Volume удалён"), wxT("Успех"), wxOK | wxICON_INFORMATION);
    }
}

void DockerManagerFrame::OnPruneAll(wxCommandEvent& event) {
    int response = wxMessageBox(
        wxT("ВНИМАНИЕ! Это удалит:\n"
            "- Все остановленные контейнеры\n"
            "- Все неиспользуемые образы\n"
            "- Все неиспользуемые volumes\n"
            "- Весь build cache\n\n"
            "Продолжить?"),
        wxT("Подтверждение полной очистки"),
        wxYES_NO | wxICON_WARNING,
        this
    );
    
    if (response == wxYES) {
        wxMessageBox(wxT("Очистка началась, это может занять время..."), 
                     wxT("Информация"), wxOK | wxICON_INFORMATION);
        ExecuteCommand("../scripts/docker_info.sh prune");
        RefreshAll();
        wxMessageBox(wxT("Очистка завершена!"), wxT("Успех"), wxOK | wxICON_INFORMATION);
    }
}

void DockerManagerFrame::OnRefresh(wxCommandEvent& event) {
    RefreshAll();
}

void DockerManagerFrame::OnTimer(wxTimerEvent& event) {
    UpdateSystemInfo();
}

void DockerManagerFrame::OnClose(wxCloseEvent& event) {
    refreshTimer->Stop();
    delete refreshTimer;
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
