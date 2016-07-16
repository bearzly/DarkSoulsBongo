/* Copyright (c) 2016 Benjamin Gwin
 * 
 * This file is licensed under the terms of the GPLv3.
 * See the included license file for details.
*/

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>

#include "BongoController.h"

// Value from clap sensor to consider a clap (range is 0-65535)
#define CLAP_THRESHOLD 15000

// Map of Xbox buttons to their corresponding DirectInput key codes
WORD Bcodes[] = {DIK_SPACE, DIK_BACKSPACE};
WORD RRcodes[] = {DIK_L};
WORD RLcodes[] = {DIK_J};
WORD RBcodes[] = {DIK_H};
WORD LUcodes[] = {DIK_W};
WORD Ycodes[] = {DIK_N, DIK_INSERT};
WORD Xcodes[] = {DIK_E, DIK_DELETE};
WORD LBcodes[] = {DIK_U};
WORD LTcodes[] = {DIK_Y};
WORD LDcodes[] = {DIK_D};
WORD DUcodes[] = {DIK_UPARROW, DIK_R};
WORD DRcodes[] = {DIK_RIGHTARROW, DIK_V};
WORD DLcodes[] = {DIK_LEFTARROW, DIK_C};
WORD DDcodes[] = {DIK_DOWNARROW, DIK_F};
WORD Acodes[] = {DIK_Q, DIK_RETURN};
WORD STARTcodes[] = {DIK_ESCAPE};
WORD R3codes[] = {DIK_O};
WORD BACKcodes[] = {DIK_G};

// Remap some strings to prettier versions
wxString leftUp = wxT("L↑");
wxString leftDown = wxT("L↓");
wxString rightRight = wxT("R→");
wxString rightLeft = wxT("R←");
wxString dpadUp = wxT("D↑");
wxString dpadRight = wxT("D→");
wxString dpadLeft = wxT("D←");
wxString dpadDown = wxT("D↓");

// Tracks the state of the virtual key presses for each button and
// sends fake keystrokes using SendInput
struct KeyState
{
	WORD* scanCodes;
	int ncodes;
	bool enabled;
	wxString label;

	KeyState(WORD* code, int ncodes, wxString label):
		scanCodes(code),
		ncodes(ncodes),
		enabled(false),
		label(label)
	{
	}

	KeyState():
		scanCodes(NULL), 
		ncodes(0), 
		enabled(false)
	{
	}

	void enable()
	{
 		enabled = true;
		for (int i = 0; i < ncodes; ++i)
		{
			INPUT input;
			input.type = INPUT_KEYBOARD;
			input.ki.wVk = scanCodes[i];
			input.ki.wScan = scanCodes[i];
			input.ki.dwFlags = KEYEVENTF_SCANCODE;
			input.ki.time = 0;
			input.ki.dwExtraInfo = NULL;

			SendInput(1, &input, sizeof(input));
		}
	}

	void disable()
	{
		if (!enabled) return;
		enabled = false;
		for (int i = 0; i < ncodes; ++i)
		{
			INPUT input;
			input.type = INPUT_KEYBOARD;
			input.ki.wVk = scanCodes[i];
			input.ki.wScan = scanCodes[i];
			input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
			input.ki.time = 0;
			input.ki.dwExtraInfo = NULL;

			SendInput(1, &input, sizeof(input));
		}
	}
};

// The bongo controller requires multiple states in order to enable
// enough actions. Each state controls 6 different key states corresponding
// to the 6 bongo inputs.
struct BongoState
{
	KeyState keys[6];
	bool enabled;

	BongoState(KeyState s1, KeyState s2, KeyState s3, KeyState s4, KeyState s5, KeyState s6):
		enabled(false)
	{
		keys[0] = s1;
		keys[1] = s2;
		keys[2] = s3;
		keys[3] = s4;
		keys[4] = s5;
		keys[5] = s6;
	}

	BongoState()
	{
	}

	void disable()
	{
		enabled = false;
	}

	void enable()
	{
		enabled = true;
	}

	void tick(bool b1, bool b2, bool b3, bool b4, bool b5, bool b6)
	{
		if (b1 && enabled)
			keys[0].enable();
		else
			keys[0].disable();
		if (b2 && enabled)
			keys[1].enable();
		else
			keys[1].disable();
		if (b3 && enabled)
			keys[2].enable();
		else
			keys[2].disable();
		if (b4 && enabled)
			keys[3].enable();
		else
			keys[3].disable();
		if (b5 && enabled)
			keys[4].enable();
		else
			keys[4].disable();
		if (b6 && enabled)
			keys[5].enable();
		else
			keys[5].disable();
	}

	const wxString& getLabel(int i)
	{
		const wxString& label = keys[i].label;
		if (label == "LU") return leftUp;
		else if (label == "LD") return leftDown;
		else if (label == "RR") return rightRight;
		else if (label == "RL") return rightLeft;
		else if (label == "DU") return dpadUp;
		else if (label == "DR") return dpadRight;
		else if (label == "DL") return dpadLeft;
		else if (label == "DD") return dpadDown;
		return label;
	}

	bool isEnabled(int i)
	{
		return keys[i].enabled;
	}
};

// States that the bongo controller state machine might be in
enum State
{
	IDLE,     // Default state when nothing has happened
	SINGLE,   // Detected that a single button was pressed
	CLAP,     // Detected a clap
	DFRONT,   // Detected that both front buttons were pressed
	DBACK,    // Detected that both back buttons were pressed
	DEBOUNCE, // Transition state to ignore accidental input on the way back to IDLE
};

class wxBongoView : public wxWindow
{
	static const int fixedWidth = 600;
	static const int fixedHeight = 400;

	BongoController bc;
	wxBrush brownBrush;
	wxTimer* m_timer;
	BongoState* states[3];
	State smState;
	int bongoState;
	wxString stateStr;
	int clapValue;
	BongoControllerState oldControllerState;
public:
    wxBongoView(wxFrame* parent);
	~wxBongoView();
 
    void paintEvent(wxPaintEvent & evt);

	void timerTick(wxTimerEvent& event);
 
    void render(wxDC& dc);
  
    DECLARE_EVENT_TABLE()
};
 
 
BEGIN_EVENT_TABLE(wxBongoView, wxPanel) 
    EVT_PAINT(wxBongoView::paintEvent)
	EVT_TIMER(wxID_HIGHEST, wxBongoView::timerTick)
END_EVENT_TABLE()
 
wxBongoView::wxBongoView(wxFrame* parent):
	wxWindow(parent, wxID_ANY),
	brownBrush(wxColour(124, 68, 0)),
	bc(this->GetHandle()),
	clapValue(0)
{
    this->SetMinSize(wxSize(fixedWidth, fixedHeight));
	this->SetBackgroundColour(*wxGREEN);
	this->SetDoubleBuffered(true);
	
	m_timer = new wxTimer(this, wxID_HIGHEST);
	m_timer->Start(33);

#define NELEMS(x) (sizeof(x)/sizeof(x[0]))
#define KS(x) KeyState(x##codes, NELEMS(x##codes), wxT(#x))

	// Declare the different states and their corresponding buttons
	states[0] = new BongoState(KS(B), KS(RR), KS(RL), KS(RB), KS(LU), KS(R3));
	states[1] = new BongoState(KS(Y), KS(X), KS(LB), KS(LT), KS(LD), KS(BACK));
	states[2] = new BongoState(KS(DU), KS(DR), KS(DL), KS(DD), KS(A), KS(START));
	smState = IDLE;
	bongoState = 0;
	states[bongoState]->enable();
}

wxBongoView::~wxBongoView()
{
	m_timer->Stop();
	delete states[0];
	delete states[1];
	delete states[2];
}

void wxBongoView::paintEvent(wxPaintEvent & evt)
{
    wxPaintDC dc(this);
    render(dc);
}
 
void wxBongoView::render(wxDC&  dc)
{
	int midy = fixedHeight / 2;
	int leftx = fixedWidth / 4 + 15;
	int rightx = fixedWidth * 3 / 4 - 15;
	int radius = fixedWidth / 4 - 30;
	int bigradius = radius + 6;
	int bridgeHeight = 80;
	
	wxGraphicsContext *gc = wxGraphicsContext::Create(this);
	if (gc)
	{
		// Draw the white outlines of the controller
		wxPen thickPen(*wxWHITE, 6);
		gc->SetPen(thickPen);
		gc->SetBrush(*wxTRANSPARENT_BRUSH);
		int startW = 30;
		int startH = 12;
		gc->DrawEllipse(leftx-radius, midy-radius, radius*2, radius*2);
		gc->DrawEllipse(rightx-radius, midy-radius, radius*2, radius*2);
		wxGraphicsPath path = gc->CreatePath();
		path.MoveToPoint(leftx-radius, midy);
		path.AddCurveToPoint(leftx-radius/2,midy+30,leftx+radius/2,midy+30, leftx+radius, midy);
		gc->StrokePath(path);
		wxGraphicsPath path2 = gc->CreatePath();
		path2.MoveToPoint(rightx-radius, midy);
		path2.AddCurveToPoint(rightx-radius/2,midy+30,rightx+radius/2,midy+30, rightx+radius, midy);
		gc->StrokePath(path2);
		gc->DrawEllipse(fixedWidth/2 - 27.5, midy+65, 55, 55);

		// Draw the clap bar, including the current clap value and a bar respresenting the threshold
		int clapBarW = 150;
		int clapBarH = 30;
		int clapBarOffset = 145;
		
		int clapLevelX = clapBarW * (double)clapValue / (1<<15);
		gc->SetBrush(*wxRED_BRUSH);
		gc->SetPen(*wxTRANSPARENT_PEN);
		gc->DrawRectangle(fixedWidth/2-clapBarW/2, midy-clapBarOffset, clapLevelX, clapBarH);
		gc->SetBrush(*wxTRANSPARENT_BRUSH);
		gc->DrawRectangle(fixedWidth/2-clapBarW/2+clapLevelX, midy-clapBarOffset, clapBarW-clapLevelX, clapBarH);
		gc->SetPen(thickPen);
		gc->SetBrush(*wxTRANSPARENT_BRUSH);
		gc->DrawRectangle(fixedWidth/2-clapBarW/2-1, midy-clapBarOffset-1, clapBarW+2, clapBarH+2);
		gc->SetPen(thickPen);
		int threshx = fixedWidth/2-clapBarW/2+(double)CLAP_THRESHOLD/(1<<15) * clapBarW;
		gc->StrokeLine(threshx, midy-clapBarOffset, threshx, midy-clapBarOffset+clapBarH);

		// Draw the text labels
		wxPoint points[] = 
		{
			wxPoint(rightx, midy-60),
			wxPoint(rightx, midy+50),
			wxPoint(leftx, midy+50),
			wxPoint(leftx, midy-60),
			wxPoint(fixedWidth/2, midy+bridgeHeight/2+30),
			wxPoint(fixedWidth/2, midy-clapBarOffset-45),
		};

		wxFont font(28, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
		for (int i = 0; i < 6; ++i)
		{
			wxString s = states[bongoState]->getLabel(i);
			gc->SetFont(font, states[bongoState]->isEnabled(i) ? *wxRED : *wxWHITE);
			double h, w;
			gc->GetTextExtent(s, &w, &h);
			gc->DrawText(s, points[i].x-w/2, points[i].y);
		}

        delete gc;
	}
	
}

#define SET_STATE(x) do { smState = x; stateStr = wxT(#x); } while (0)

void wxBongoView::timerTick(wxTimerEvent& evt)
{
	// Query the bongo controller's current state and handle accordingly
	BongoControllerState st;
	bc.getState(st);

	State oldState = smState;
	switch (smState)
	{
	case IDLE:
		if (st.b2 && st.b3)
		{
			SET_STATE(DFRONT);
			if (bongoState >= 2)
				break;
			states[bongoState]->disable();
			bongoState++;
			states[bongoState]->enable();
		}
		else if (st.b1 && st.b4)
		{
			SET_STATE(DBACK);
			if (bongoState <= 0)
				break;
			states[bongoState]->disable();
			bongoState--;
			states[bongoState]->enable();
		}
		else if (st.any())
		{
			SET_STATE(SINGLE);
		}
		else if (st.clap > CLAP_THRESHOLD)
		{
			SET_STATE(CLAP);
		}
		break;
	case SINGLE:
		if (!st.any())
			SET_STATE(DEBOUNCE);
		break;
	case CLAP:
		SET_STATE(IDLE);
		break;
	case DFRONT:
		if (!st.any())
			SET_STATE(DEBOUNCE);
		break;
	case DBACK:
		if (!st.any())
			SET_STATE(DEBOUNCE);
		break;
	case DEBOUNCE:
		SET_STATE(IDLE);
		break;
	}
	if (smState == SINGLE)
		states[bongoState]->tick(st.b1, st.b2, st.b3, st.b4, st.b5, false);
	else
		states[bongoState]->tick(false, false, false, false, false, false);
	if (smState == CLAP)
		states[bongoState]->tick(false, false, false, false, false, true);
	
	clapValue = st.clap;
	if (smState != oldState || !(oldControllerState == st))
		Refresh();
	oldControllerState = st;
}
 
 
class BongoApp : public wxApp
{
 
    wxFrame *frame;
    wxBongoView* view;
 
public:
 
    bool OnInit()
    {
        wxInitAllImageHandlers();
 
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		frame = new wxFrame(NULL, wxID_ANY, wxT("Bongo GUI"), wxPoint(50,50), wxSize(800,600));
 
        view = new wxBongoView(frame);
        sizer->Add(view, 0, wxALL, 5);
 
        frame->SetSizer(sizer);
 
        frame->Show();
        return true;
    } 
 
};
 
IMPLEMENT_APP(BongoApp)