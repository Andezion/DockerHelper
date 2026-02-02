#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/timer.h>
#include <wx/thread.h>
#include "docker_commands.h"

class DockerManagerFrame : public wxFrame {
public:
    DockerManagerFrame(const wxString& title);
    ~DockerManagerFrame();
    
    void OnUpdateComplete(wxThreadEvent& event);
    
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
    wxButton* stopAllButton;
    wxButton* removeContainerButton;
    wxButton* removeImageButton;
    wxButton* removeVolumeButton;
    wxButton* pruneAllButton;
    wxButton* refreshButton;
    
    wxTimer* refreshTimer;
    bool isUpdating;
    
    void CreateSystemInfoPanel(wxPanel* parent, wxSizer* sizer);
    void CreateRunningPanel();
    void CreateCleanupPanel();
    
    void PopulateRunningContainers(const std::vector<ContainerInfo>& containers);
    void PopulateStoppedContainers(const std::vector<ContainerInfo>& containers);
    void PopulateUnusedImages(const std::vector<ImageInfo>& images);
    void PopulateUnusedVolumes(const std::vector<VolumeInfo>& volumes);
    void UpdateSystemInfoUI(const SystemInfo& info);
    void RefreshAllAsync();
    
    void OnStop(wxCommandEvent& event);
    void OnStopAll(wxCommandEvent& event);
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
    
    wxDECLARE_EVENT_TABLE();
};

enum {
    ID_STOP = wxID_HIGHEST + 1,
    ID_STOP_ALL,
    ID_REMOVE_CONTAINER,
    ID_REMOVE_IMAGE,
    ID_REMOVE_VOLUME,
    ID_PRUNE_ALL,
    ID_REFRESH,
    ID_TIMER,
    ID_RUNNING_LIST,
    ID_STOPPED_LIST,
    ID_IMAGES_LIST,
    ID_VOLUMES_LIST,
    ID_UPDATE_COMPLETE
};

class DockerManagerApp : public wxApp {
public:
    virtual bool OnInit();
};

wxDECLARE_EVENT(wxEVT_UPDATE_COMPLETE, wxThreadEvent);
