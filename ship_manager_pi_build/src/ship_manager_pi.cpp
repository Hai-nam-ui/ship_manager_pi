#include "ship_manager_pi.h"
#include <wx/listctrl.h>
#include <wx/dialog.h>
#include <wx/filefn.h>
#include <wx/bitmap.h>
#include <wx/mstream.h>

// Biểu tượng plugin (dữ liệu base64, thay bằng tệp PNG thực tế)
static const unsigned char icon_data[] = {
    // Dữ liệu base64 của một biểu tượng PNG 32x32, bạn cần thay bằng tệp thực
    // Để đơn giản, tôi giả định bạn sẽ thêm tệp PNG riêng
};
static wxBitmap LoadIcon() {
    // Thay bằng đường dẫn đến ship_manager_pi_icon.png
    return wxBitmap(wxT("ship_manager_pi_icon.png"), wxBITMAP_TYPE_PNG);
}

class ShipListDialog : public wxDialog {
public:
    ShipListDialog(wxWindow *parent, std::vector<ShipInfo> &ships)
        : wxDialog(parent, wxID_ANY, wxT("Ship List"), wxDefaultPosition, wxSize(600, 400)) {
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        m_list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
        m_list->InsertColumn(0, wxT("MMSI"), wxLIST_FORMAT_LEFT, 100);
        m_list->InsertColumn(1, wxT("Name"), wxLIST_FORMAT_LEFT, 150);
        m_list->InsertColumn(2, wxT("Latitude"), wxLIST_FORMAT_LEFT, 100);
        m_list->InsertColumn(3, wxT("Longitude"), wxLIST_FORMAT_LEFT, 100);
        m_list->InsertColumn(4, wxT("Speed"), wxLIST_FORMAT_LEFT, 80);
        m_list->InsertColumn(5, wxT("Course"), wxLIST_FORMAT_LEFT, 80);

        for (size_t i = 0; i < ships.size(); i++) {
            m_list->InsertItem(i, wxString::Format(wxT("%d"), ships[i].mmsi));
            m_list->SetItem(i, 1, ships[i].name);
            m_list->SetItem(i, 2, wxString::Format(wxT("%.4f"), ships[i].lat));
            m_list->SetItem(i, 3, wxString::Format(wxT("%.4f"), ships[i].lon));
            m_list->SetItem(i, 4, wxString::Format(wxT("%.1f"), ships[i].speed));
            m_list->SetItem(i, 5, wxString::Format(wxT("%.1f"), ships[i].course));
        }

        sizer->Add(m_list, 1, wxEXPAND | wxALL, 5);
        SetSizer(sizer);
    }

private:
    wxListCtrl *m_list;
};

ShipManagerPlugin::ShipManagerPlugin(void *ppimgr) : opencpn_plugin_118(ppimgr) {
    m_bitmap = LoadIcon();
    m_data_file = GetPluginDataDir("ship_manager_pi") + wxFileName::GetPathSeparator() + wxT("ship_manager_pi.json");
}

ShipManagerPlugin::~ShipManagerPlugin() {}

int ShipManagerPlugin::Init(void) {
    AddToolbarToolId(-1, wxT("ShipManager"), m_bitmap);
    m_parent_window = GetOCPNCanvasWindow();
    LoadShipsFromFile();
    return (WANTS_TOOLBAR_CALLBACK | WANTS_NMEA_SENTENCES | INSTALLS_TOOLBAR_TOOL);
}

bool ShipManagerPlugin::DeInit(void) {
    SaveShipsToFile();
    return true;
}

void ShipManagerPlugin::OnToolbarToolCallback(int id) {
    ShowShipListDialog();
}

void ShipManagerPlugin::ShowShipListDialog() {
    ShipListDialog dlg(m_parent_window, m_ships);
    dlg.ShowModal();
}

bool ShipManagerPlugin::ParseNMEAAIS(const wxString &sentence, ShipInfo &ship) {
    // Phân tích cơ bản câu !AIVDM (giả lập, cần thư viện NMEA đầy đủ để xử lý thực tế)
    wxStringTokenizer tokenizer(sentence, wxT(","));
    if (tokenizer.CountTokens() < 6) return false;

    // Giả lập dữ liệu AIS
    tokenizer.GetNextToken(); // Bỏ qua các trường không cần thiết
    ship.mmsi = wxAtoi(tokenizer.GetNextToken());
    ship.name = tokenizer.GetNextToken();
    ship.lat = wxAtof(tokenizer.GetNextToken());
    ship.lon = wxAtof(tokenizer.GetNextToken());
    ship.speed = wxAtof(tokenizer.GetNextToken());
    ship.course = wxAtof(tokenizer.GetNextToken());
    return true;
}

void ShipManagerPlugin::SetNMEASentence(wxString &sentence) {
    if (sentence.StartsWith(wxT("!AIVDM"))) {
        ShipInfo ship;
        if (ParseNMEAAIS(sentence, ship)) {
            // Cập nhật hoặc thêm tàu
            for (auto &s : m_ships) {
                if (s.mmsi == ship.mmsi) {
                    s = ship;
                    return;
                }
            }
            m_ships.push_back(ship);
            SaveShipsToFile();
        }
    }
}

void ShipManagerPlugin::SaveShipsToFile() {
    wxJSONValue root;
    wxJSONValue ships;
    for (const auto &ship : m_ships) {
        wxJSONValue ship_data;
        ship_data[wxT("mmsi")] = ship.mmsi;
        ship_data[wxT("name")] = ship.name;
        ship_data[wxT("lat")] = ship.lat;
        ship_data[wxT("lon")] = ship.lon;
        ship_data[wxT("speed")] = ship.speed;
        ship_data[wxT("course")] = ship.course;
        ships.Append(ship_data);
    }
    root[wxT("ships")] = ships;

    wxJSONWriter writer;
    wxString json_str;
    writer.Write(root, json_str);

    wxFile file(m_data_file, wxFile::write);
    file.Write(json_str);
    file.Close();
}

void ShipManagerPlugin::LoadShipsFromFile() {
    if (!wxFileExists(m_data_file)) return;

    wxFile file(m_data_file, wxFile::read);
    wxString json_str;
    file.ReadAll(&json_str);
    file.Close();

    wxJSONValue root;
    wxJSONReader reader;
    reader.Parse(json_str, &root);

    m_ships.clear();
    wxJSONValue ships = root[wxT("ships")];
    for (int i = 0; i < ships.Size(); i++) {
        ShipInfo ship;
        ship.mmsi = ships[i][wxT("mmsi")].AsInt();
        ship.name = ships[i][wxT("name")].AsString();
        ship.lat = ships[i][wxT("lat")].AsDouble();
        ship.lon = ships[i][wxT("lon")].AsDouble();
        ship.speed = ships[i][wxT("speed")].AsDouble();
        ship.course = ships[i][wxT("course")].AsDouble();
        m_ships.push_back(ship);
    }
}