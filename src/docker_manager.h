#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/timer.h>
#include <string>

struct ContainerInfo {
    std::string id;
    std::string name;
    std::string status;
    std::string image;
};

struct SystemInfo {
    std::string cpu_usage;
    std::string mem_usage;
    int container_count;
};

class DockerManagerFrame : public wxFrame {
public:
    DockerManagerFrame(const wxString& title);
    
private:
    wxNotebook* notebook;
    wxPanel* runningPanel;
    wxPanel* cleanupPanel;
    
    wxListCtrl* runningList;
    wxListCtrl* stoppedList;
    wxListCtrl* imagesList;
    wxListCtrl* volumesList;
    
    wxStaticText* cpuLabel;
    wxStaticText* memLabel;
    wxStaticText* containersLabel;
    
    wxButton* stopButton;
    wxButton* removeContainerButton;
    wxButton* removeImageButton;
    wxButton* removeVolumeButton;
    wxButton* pruneAllButton;
    wxButton* refreshButton;
    
    wxTimer* refreshTimer;
    
    std::string ExecuteCommand(const std::string& command);
    void UpdateRunningContainers();
    void UpdateStoppedContainers();
    void UpdateUnusedImages();
    void UpdateUnusedVolumes();
    void UpdateSystemInfo();
    void RefreshAll();
    
    void OnStop(wxCommandEvent& event);
    void OnRemoveContainer(wxCommandEvent& event);
    void OnRemoveImage(wxCommandEvent& event);
    void OnRemoveVolume(wxCommandEvent& event);
    void OnPruneAll(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnRunningItemSelected(wxListEvent& event);
    void OnStoppedItemSelected(wxListEvent& event);
    void OnImageItemSelected(wxListEvent& event);
    void OnVolumeItemSelected(wxListEvent& event);
    
    void CreateRunningPanel();
    void CreateCleanupPanel();
    void CreateSystemInfoPanel(wxPanel* parent, wxSizer* sizer);
    
    wxDECLARE_EVENT_TABLE();
};

enum {
    ID_STOP = wxID_HIGHEST + 1,
    ID_REMOVE_CONTAINER,
    ID_REMOVE_IMAGE,
    ID_REMOVE_VOLUME,
    ID_PRUNE_ALL,
    ID_REFRESH,
    ID_TIMER,
    ID_RUNNING_LIST,
    ID_STOPPED_LIST,
    ID_IMAGES_LIST,
    ID_VOLUMES_LIST
};

class DockerManagerApp : public wxApp {
public:
    virtual bool OnInit();
};

