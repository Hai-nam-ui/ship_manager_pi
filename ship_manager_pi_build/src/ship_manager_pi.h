#ifndef _SHIP_MANAGER_PI_H_
#define _SHIP_MANAGER_PI_H_

#include "ocpn_plugin.h"
#include <wx/wx.h>
#include <wx/file.h>
#include <wx/jsonreader.h>
#include <wx/jsonwriter.h>
#include <vector>

struct ShipInfo {
    int mmsi;
    double lat;
    double lon;
    double speed;
    double course;
    wxString name;
};

class ShipManagerPlugin : public opencpn_plugin_118 {
public:
    ShipManagerPlugin(void *ppimgr);
    ~ShipManagerPlugin();

    int Init(void) override;
    bool DeInit(void) override;
    int GetAPIVersionMajor() override { return OCPN_API_VERSION_MAJOR; }
    int GetAPIVersionMinor() override { return OCPN_API_VERSION_MINOR; }
    int GetPlugInVersionMajor() override { return 1; }
    int GetPlugInVersionMinor() override { return 0; }
    wxBitmap *GetPlugInBitmap() override { return &m_bitmap; }
    wxString GetCommonName() override { return wxT("Ship Manager"); }
    wxString GetShortDescription() override { return wxT("Manage ships from AIS data"); }
    wxString GetLongDescription() override { return wxT("A plugin to manage and display ship information from AIS, with data persistence."); }

    void OnToolbarToolCallback(int id) override;
    void SetNMEASentence(wxString &sentence) override;

private:
    wxWindow *m_parent_window;
    wxBitmap m_bitmap;
    std::vector<ShipInfo> m_ships;
    wxString m_data_file;
    
    void ShowShipListDialog();
    void SaveShipsToFile();
    void LoadShipsFromFile();
    bool ParseNMEAAIS(const wxString &sentence, ShipInfo &ship);
};

#endif