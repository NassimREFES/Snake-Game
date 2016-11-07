#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/wfstream.h>
#include <wx/dcbuffer.h>
#include <wx/dc.h>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <exception>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <set>
#include <limits>
#include <random>     
#include <chrono> 

using namespace std;

int random(int min, int max)
{
	return min + rand() % (max - min + 1);
}

class Snake : public wxFrame
{
public:
	enum Snake_direction { LEFT, RIGHT, UP, DOWN };
	Snake();

protected:
	void OnPaint(wxPaintEvent& event);
	void PaintBackground(wxDC& dc);

	void OnKeyDown(wxKeyEvent&);
	void OnTimer(wxCommandEvent&);

	void OnQuit(wxCommandEvent&);

private:
	wxSize paint_zone;

	wxMenuBar* m_menubar;
	wxMenu* m_fichier;
	wxMenuItem* m_quit;

	wxPoint begin_grid;
	wxPoint end_grid;
	int xstep, ystep;

	vector<wxPoint> vsnake;
	wxPoint target;

	wxTimer *timer;
	Snake_direction sd;
	bool start;
	bool pause;
	bool finish;
	int count;

private:
	void draw_grid(wxBufferedPaintDC&, const wxPoint&, const wxPoint&);
	void draw_snake(wxBufferedPaintDC&);
	void next_target(const wxPoint&, const wxPoint&);
	void draw_target(wxBufferedPaintDC&);
	bool collision_target_with_snake(const wxPoint& targetx, const int from = 0, bool first_only = false) const;
	void move_snake(Snake_direction);
};

Snake::Snake()
: wxFrame(NULL, wxID_ANY, wxT("Simple Snake"), wxDefaultPosition, wxSize(1100, 800)),
paint_zone(1100, 800), count(0)
{
	wxImage::AddHandler(new wxPNGHandler);
	m_menubar = new wxMenuBar;
	m_fichier = new wxMenu;
	m_quit = new wxMenuItem(m_fichier, wxID_EXIT, wxT("&Quit"));
	m_fichier->Append(m_quit);
	m_menubar->Append(m_fichier, wxT("&Fichier"));
	SetMenuBar(m_menubar);
	// ------------------------------------------------------------
	begin_grid = wxPoint(100, 100);
	end_grid = wxPoint(paint_zone.GetWidth() - 150, paint_zone.GetHeight() - 150);
	xstep = (end_grid.x - begin_grid.x) / 25;
	ystep = (end_grid.y - begin_grid.y) / 25;

	wxPoint start_snake(begin_grid.x + xstep, begin_grid.y + ystep * 12);
	vsnake.push_back(start_snake);
	vsnake.push_back(wxPoint(begin_grid.x, begin_grid.y + ystep * 12));

	timer = new wxTimer(this, 1);
	start = false;
	pause = false;
	finish = false;

	sd = Snake_direction::RIGHT;

	next_target(begin_grid, end_grid);
	//SetBackgroundColour(*wxWHITE);
	Connect(wxEVT_PAINT, wxPaintEventHandler(Snake::OnPaint));
	Connect(wxID_EXIT, wxEVT_MENU, wxCommandEventHandler(Snake::OnQuit));

	Connect(wxEVT_TIMER, wxCommandEventHandler(Snake::OnTimer));
	Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(Snake::OnKeyDown));
}

void Snake::move_snake(Snake_direction sd1)
{
	wxPoint prev = vsnake[0];
	switch (sd1) {
		case Snake_direction::LEFT: { vsnake[0].x = vsnake[0].x - xstep; } break;
		case Snake_direction::RIGHT: { vsnake[0].x = vsnake[0].x + xstep; } break;
		case Snake_direction::UP: { vsnake[0].y = vsnake[0].y - ystep; } break;
		case Snake_direction::DOWN: { vsnake[0].y = vsnake[0].y + ystep; } break;
	}

	// colision avec sois meme
	for (int i = 1; i < vsnake.size(); ++i)
	if (collision_target_with_snake(vsnake[0], 1)) {
		finish = true;
		wxMessageBox(wxT("=====> LOST GAME: snake collision <====="), wxT("END"));
		return;
	}

	wxPoint prev_temp;
	for (int i = 1; i < vsnake.size(); ++i) {
		prev_temp = vsnake[i];
		vsnake[i] = prev;
		prev = prev_temp;
	}

	if (vsnake[0].x > begin_grid.x + xstep * 25) vsnake[0].x = begin_grid.x;
	else if (vsnake[0].x < begin_grid.x) vsnake[0].x = begin_grid.x + xstep * 25;
	if (vsnake[0].y > begin_grid.y + ystep * 25) vsnake[0].y = begin_grid.y;
	else if (vsnake[0].y < begin_grid.y) vsnake[0].y = begin_grid.y + ystep * 25;
}

void Snake::OnTimer(wxCommandEvent& event)
{
	if (!pause && !finish)
		move_snake(sd);
	if (collision_target_with_snake(target)) {
		vsnake.push_back(vsnake[vsnake.size() - 1]);
		next_target(begin_grid, end_grid);
		++count;
	}
	Refresh();
}

void Snake::OnKeyDown(wxKeyEvent& event)
{
	int key = event.GetKeyCode();

	switch (key) {
		case 's': case 'S': if (!start) { start = true; pause = false; timer->Start(300); } return;
		case 'p': case 'P': if (!pause) { pause = true; start = false; timer->Stop(); } return;
		case WXK_LEFT	: sd = Snake_direction::LEFT; break;
		case WXK_RIGHT	: sd = Snake_direction::RIGHT; break;
		case WXK_UP		: sd = Snake_direction::UP; break;
		case WXK_DOWN	: sd = Snake_direction::DOWN; break;
		default : event.Skip();
	}
}

void Snake::PaintBackground(wxDC& dc)
{
	wxColour backgroundColour = GetBackgroundColour();
	if (!backgroundColour.IsOk())
		backgroundColour = /*wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE)*/*wxWHITE;
	dc.SetBrush(wxBrush(backgroundColour));
	dc.SetPen(wxPen(backgroundColour, 1));
	wxRect windowRect(wxPoint(0, 0), GetClientSize());
	/*CalcUnscrolledPosition(windowRect.x, windowRect.y,
	&windowRect.x, &windowRect.y);*/
	dc.DrawRectangle(windowRect);
}

void Snake::draw_snake(wxBufferedPaintDC& dc)
{
	dc.SetBrush(*wxBLUE_BRUSH);
	dc.SetPen(wxPen(*wxBLUE, 4, wxPENSTYLE_SOLID));
	dc.DrawCircle(vsnake[0], 8); // snake head 

	dc.SetBrush(*wxBLUE_BRUSH);
	dc.SetPen(wxPen(*wxBLUE, 6, wxPENSTYLE_SOLID));

	for (int i = 1; i < vsnake.size(); ++i) {
		if (abs(vsnake[i - 1].x - vsnake[i].x) == xstep)
			dc.DrawLine(vsnake[i - 1], vsnake[i]);
		if (abs(vsnake[i - 1].y - vsnake[i].y) == ystep)
			dc.DrawLine(vsnake[i - 1], vsnake[i]);
	}
}

bool Snake::collision_target_with_snake(const wxPoint& targetx, const int from, bool first_only) const
{
	if (first_only || vsnake.size() == 1) return vsnake[0].x == targetx.x && vsnake[0].y == targetx.y;
	for (int i = from; i < vsnake.size(); ++i)
		if (vsnake[i].x == targetx.x && vsnake[i].y == targetx.y)
			return true;
	return false;
}

void Snake::draw_target(wxBufferedPaintDC& dc)
{
	dc.SetBrush(*wxRED_BRUSH);
	dc.SetPen(wxPen(*wxRED, 3, wxPENSTYLE_SOLID));
	dc.DrawCircle(target, 5);
}

void Snake::next_target(const wxPoint& pa, const wxPoint& pb)
{
	do {
		target.x = pa.x + random(0, 25) * xstep;
		target.y = pa.y + random(0, 25) * ystep;
	} while (collision_target_with_snake(target));
}

void Snake::draw_grid(wxBufferedPaintDC& dc, const wxPoint& pa, const wxPoint& pb)
{
	dc.SetBrush(*wxWHITE_BRUSH);
	dc.SetPen(wxPen(*wxBLACK, 2, wxPENSTYLE_SOLID));

	for (int i = pa.y; i < pb.y; i += ystep)
		dc.DrawLine(wxPoint(pa.x, i), wxPoint(pb.x, i));
	for (int i = pa.x; i < pb.x; i += xstep)
		dc.DrawLine(wxPoint(i, pa.y), wxPoint(i, pb.y));
}

void Snake::OnPaint(wxPaintEvent& event)
{
	wxBufferedPaintDC dc(this);

	dc.SetBrush(*wxBLACK_BRUSH);
	dc.SetPen(wxPen(*wxBLUE, 4, wxPENSTYLE_SOLID));
	dc.DrawRectangle(wxPoint(0, 0), paint_zone);
	dc.SetBrush(*wxWHITE_BRUSH);
	dc.SetPen(wxPen(*wxBLACK, 4, wxPENSTYLE_SOLID));
	dc.DrawRectangle(wxPoint(100, 100), wxSize(paint_zone.GetWidth() - 250, paint_zone.GetHeight() - 250));

	draw_grid(dc, begin_grid, end_grid);
	if (!finish && !pause) {
		draw_target(dc);
		draw_snake(dc);
	}

	dc.SetBrush(*wxWHITE_BRUSH);
	dc.SetPen(wxPen(*wxRED, 4, wxPENSTYLE_SOLID));
	dc.DrawRectangle(wxPoint(10, 10), wxSize(80, 80));

	wxString x;
	x << count;
	wxFont font(40, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false);
	dc.SetFont(font);
	dc.SetTextForeground(wxColour(221, 34, 34));
	dc.DrawText(x, wxPoint(10, 10));

	dc.SetPen(wxNullPen);
}

void Snake::OnQuit(wxCommandEvent& event)
{
	Close(true);
}

class MyApp : public wxApp
{
public:
	virtual bool OnInit();
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
	srand(time(NULL));
	Snake* s = new Snake();
	s->Centre();
	s->Show(true);

	return true;
}
