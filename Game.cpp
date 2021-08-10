/*
 * Game.cpp
 *
 *  Created on: 29/mar/2015
 *      Author: stefano
 */

#include "Game.h"

#include "2DAResource.h"
#include "Actor.h"
#include "Core.h"
#include "CreResource.h"
#include "Dialog.h"
#include "GraphicsEngine.h"
#include "GUI.h"
#include "InputConsole.h"
#include "Parsing.h"
#include "Party.h"
#include "OutputConsole.h"
#include "ResManager.h"
#include "RoomBase.h"
#include "TextArea.h"
#include "Timer.h"


#include <assert.h>
#include <stdio.h>

static Game* sGame;

/* static */
Game*
Game::Get()
{
	if (sGame == NULL)
		sGame = new Game();
	return sGame;
}


Game::Game()
	:
	fDialog(NULL),
	fParty(NULL),
	fTestMode(false)
{
	fParty = new ::Party();
}


Game::~Game()
{
	TerminateDialog();
	delete fParty;
}


uint32
DisplayClock(uint32 interval, void *param)
{
	std::string clock = GameTimer::GameTimeString();
	GUI::Get()->DisplayString(clock.c_str(), 200, 200, 2500);
	return interval;
}


void
Game::Loop(bool noNewGame, bool executeScripts)
{
	std::cout << "Game::Loop()" << std::endl;
	if (!GUI::Initialize(GraphicsEngine::Get()->ScreenFrame().w,
						 GraphicsEngine::Get()->ScreenFrame().h)) {
		throw std::runtime_error("Initializing GUI failed");
	}

	GFX::rect screenRect = GraphicsEngine::Get()->ScreenFrame();
	GUI* gui = GUI::Get();
	GFX::rect consoleRect(
			0,
			0,
			screenRect.w,
			screenRect.h - 21);

	std::cout << "Setting up output console... ";
	std::flush(std::cout);
	OutputConsole console(consoleRect, false);
	consoleRect.h = 20;
	consoleRect.y = screenRect.h - consoleRect.h;
	std::cout << "OK!" << std::endl;
	std::cout << "Setting up input console... ";
	std::flush(std::cout);
	InputConsole inputConsole(consoleRect);
	inputConsole.Initialize();
	std::cout << "OK!" << std::endl;

	if (TestMode()) {
		GUI::Get()->Load("GUITEST");
	} else {
		// TODO: Move this elsewhere.
		// This should be filled by the player selection
		IE::point point = { 20, 20 };
		try {
			if (fParty->CountActors() == 0) {
				if (Core::Get()->Game() == GAME_BALDURSGATE) {
					fParty->AddActor(new Actor("AJANTI", point, 0));
				} else {
					fParty->AddActor(new Actor("ANOMEN10", point, 0));
				}
			}
		} catch (...) {
			throw std::runtime_error("Error creating player!");
		}
		if (noNewGame)
			Core::Get()->LoadWorldMap();
		else
			LoadStartingArea();
	}
	std::cout << "Game: Started game loop." << std::endl;
	SDL_Event event;
	bool quitting = false;

	Timer::AddPeriodicTimer((uint32)8000, DisplayClock, NULL);

	while (!quitting) {
		uint32 startTicks = Timer::Ticks();
		while (SDL_PollEvent(&event) != 0) {
			RoomBase* room = Core::Get()->CurrentRoom();
			switch (event.type) {
				case SDL_MOUSEBUTTONDOWN:
					gui->MouseDown(event.button.x, event.button.y);
					break;
				case SDL_MOUSEBUTTONUP:
					gui->MouseUp(event.button.x, event.button.y);
					break;
				case SDL_MOUSEMOTION:
					gui->MouseMoved(event.motion.x, event.motion.y);
					break;
				case SDL_KEYDOWN: {
					if (event.key.keysym.sym == SDLK_ESCAPE) {
						console.Toggle();
						inputConsole.Toggle();
					} else if (console.IsActive()) {
						if (event.key.keysym.scancode < 0x80 && event.key.keysym.scancode > 0) {
							uint8 key = event.key.keysym.sym;
							if (event.key.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT))
								key -= 32;
							inputConsole.HandleInput(key);
						}
					} else {
						switch (event.key.keysym.sym) {
							case SDLK_o:
								if (room != NULL)
									room->ToggleOverlays();
								break;
							case SDLK_d:
								if (console.HasOutputRedirected())
									console.DisableRedirect();
								else
									console.EnableRedirect();
								break;
							// TODO: Move to GUI class
							case SDLK_h:
								if (room != NULL)
									room->ToggleGUI();
								break;
							case SDLK_a:
								if (room != NULL)
									room->ToggleAnimations();
								break;
							case SDLK_p:
								if (room != NULL)
									room->TogglePolygons();
								break;
							case SDLK_w:
								Core::Get()->LoadWorldMap();
								break;
							case SDLK_s:
								if (room != NULL)
									room->ToggleSearchMap();
								break;
							case SDLK_n:
								ToggleDayNight();
								break;
							case SDLK_q:
								quitting = true;
								break;
							case SDLK_1:
								GUI::Get()->ToggleWindow(1);
								break;
							case SDLK_2:
								GUI::Get()->ToggleWindow(2);
								break;
							case SDLK_3:
								GUI::Get()->ToggleWindow(3);
								break;
							case SDLK_4:
								GUI::Get()->ToggleWindow(4);
								break;
							case SDLK_SPACE:
								Core::Get()->TogglePause();
								break;
							default:
								break;
						}
					}
				}
				break;

				case SDL_QUIT:
					quitting = true;
					break;
				default:
					break;
			}
		}

		gui->Draw();

		console.Draw();
		inputConsole.Draw();
		if (!TestMode())
			Core::Get()->UpdateLogic(executeScripts);
		GraphicsEngine::Get()->Update();
		
		Timer::WaitSync(startTicks, 35);
	}

	std::cout << "Game: Input loop stopped." << std::endl;

	GUI::Destroy();
}


void
Game::InitiateDialog(Actor* actor, Actor* target)
{
	assert(fDialog == NULL);

	const res_ref dialogFile = actor->CRE()->DialogFile();
	if (dialogFile.name[0] == '\0'
			|| strcasecmp(dialogFile.CString(), "None") == 0) {
		std::cout << "EMPTY DIALOG FILE" << std::endl;
		return;
	}

	GUI::Get()->EnsureShowDialogArea();

	trigger_entry triggerEntry("LastTalkedToBy", actor);
	actor->AddTrigger(triggerEntry);
	std::cout << "initiates dialog with " << actor->LongName() << std::endl;
	std::cout << "Dialog file: " << dialogFile << std::endl;

	fDialog = new DialogHandler(actor, target, dialogFile);
	HandleDialog();
}


bool
Game::InDialogMode() const
{
	return fDialog != NULL;
}


void
Game::TerminateDialog()
{
	// Called by Core::TerminateDialog().
	// TODO: If called from other places, Core will still be in dialog mode
	if (InDialogMode()) {
		std::cout << fDialog->Actor()->Name() << " TerminateDialog()" << std::endl;
		fDialog->Actor()->IncrementNumTimesTalkedTo();
	}
	delete fDialog;
	fDialog = NULL;
}


DialogHandler*
Game::Dialog()
{
	return fDialog;
}


void
Game::HandleDialog()
{
	DialogHandler::State* currentState = fDialog->GetNextValidState();
	if (currentState == NULL) {
		std::cout << "Game::HandleDialog(): GetNextState is NULL" << std::endl;
		TerminateDialog();
		return;
	}

	// TODO: handle all transitions
	// present options to the player
	// etc.
	TextArea* textArea = GUI::Get()->GetMessagesTextArea();
	if (textArea == NULL) {
		std::cerr << "NULL Text Area!!!" << std::endl;
		return;
	}

	std::cout << "Display message." << std::endl;
	Core::Get()->DisplayMessage(fDialog->Actor()->LongName().c_str(), currentState->Text().c_str());

	std::cout << "Getting transitions..." << std::endl;
	for (int32 t = 0; t < fDialog->CountTransitions(); t++) {
		DialogHandler::Transition transition = fDialog->TransitionAt(t);
		if (!transition.text_player.empty()) {
			std::string option("-");
			textArea->AddDialogText(option.c_str(), transition.text_player.c_str(), t);
		}
	}
}


Party*
Game::Party()
{
	return fParty;
}


void
Game::LoadStartingArea()
{
	std::cout << "Load Starting Area...";
	TWODAResource* resource = gResManager->Get2DA("STARTARE");
	if (resource == NULL) {
		std::cout << "Failed!" << std::endl;
		return;
	}

	std::cout << "OK!" << std::endl;
	std::string startingArea = resource->ValueFor("START_AREA", "VALUE");
	IE::point viewPosition;
	viewPosition.x = resource->IntegerValueFor("START_XPOS", "VALUE");
	viewPosition.y = resource->IntegerValueFor("START_YPOS", "VALUE");

	std::cout << "Starting area: " << startingArea << std::endl;
	std::cout << "Starting position: " << viewPosition.x << "," << viewPosition.y << std::endl;
	
	TWODAResource* startPosResource = gResManager->Get2DA("STARTPOS");
	if (startPosResource == NULL) {
		std::cout << "Failed!" << std::endl;
		gResManager->ReleaseResource(resource);
		return;
	}

	// TODO: Needed for the initial script
	if (Core::Get()->Game() == GAME_BALDURSGATE2) {
		IE::point startPoint = {20, 20};
		fParty->AddActor(new Actor("IMOEN", startPoint, 0));
	}

	Core::Get()->LoadArea(startingArea.c_str(), "foo", NULL);
	Core::Get()->CurrentRoom()->SetAreaOffsetCenter(viewPosition);

	if (fParty != NULL) {
		for (int16 i = 0; i < fParty->CountActors(); i++) {
			char column[2];
			snprintf(column, sizeof(column), "%d", i + 1);			
			IE::point startPos;
			startPos.x = startPosResource->IntegerValueFor("START_XPOS", column);
			startPos.y = startPosResource->IntegerValueFor("START_YPOS", column);
			fParty->ActorAt(i)->SetPosition(startPos);
			fParty->ActorAt(i)->ClearDestination();
		}
	}
	
	gResManager->ReleaseResource(resource);	
	gResManager->ReleaseResource(startPosResource);
}


void
Game::ToggleDayNight()
{
	// Advance 10 hours
	GameTimer::AdvanceTime(10 * 60 * 60);

	RoomBase* area = Core::Get()->CurrentRoom();
	if (area != NULL)
		area->ReloadArea();
	// TODO: Update Area
}


void
Game::SetTestMode(bool value)
{
	fTestMode = value;
}


bool
Game::TestMode() const
{
	return fTestMode;
}
