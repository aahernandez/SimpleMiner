#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"

App* g_theApp = nullptr;

const float MIN_FRAMES_PER_SECOND = 10.f;
const float MAX_FRAMES_PER_SECOND = 60.f;
const float MIN_SECONDS_PER_FRAME = (1.f / MAX_FRAMES_PER_SECOND);
const float MAX_SECONDS_PER_FRAME = (1.f / MIN_FRAMES_PER_SECOND);

App::App()
	: isQuitting(false)
{
	g_theInput = new InputSystem();
	g_theRenderer = new Renderer();
	g_theAudio = new AudioSystem();
	g_theGame = new Game();
	g_theInput->SetMouseCursorHiddenWhenWeAreFocused(true);
}

App::~App()
{
	delete g_theGame;
	g_theGame = nullptr;

	delete g_theAudio;
	g_theAudio = nullptr;

	delete g_theRenderer;
	g_theRenderer = nullptr;

	delete g_theInput;
	g_theInput = nullptr;
}

void App::RunFrame()
{
	Input();
	float deltaSeconds = GetDeltaSeconds();
// 	if (!g_theInput->DoesAppHaveFocus())
// 		SleepSeconds(1);
	Update(deltaSeconds);
	Render();
}

void App::Update(float deltaSeconds)
{
	if (g_theGame)
		g_theGame->Update(deltaSeconds);

	if (g_theGame->IsQuitting())
		OnExitRequested();
}

void App::Input()
{
	if (g_theInput)
		g_theInput->UpdateInputState();
}

void App::OnExitRequested()
{
	isQuitting = true;
}

bool App::IsQuitting()
{
	return isQuitting;
}

void App::Render() const
{
	if (g_theGame)
		g_theGame->Render();
}

float App::GetDeltaSeconds()
{
	double timeNow = GetCurrentTimeSeconds();
	static double lastFrameTime = timeNow;
	double deltaSeconds = timeNow - lastFrameTime;

	while (deltaSeconds < MIN_SECONDS_PER_FRAME * 0.999f)
	{
		timeNow = GetCurrentTimeSeconds();
		deltaSeconds = timeNow - lastFrameTime;
		SleepSeconds(.001f);
	}
	lastFrameTime = timeNow;

	if (deltaSeconds > MAX_SECONDS_PER_FRAME)
		deltaSeconds = MAX_SECONDS_PER_FRAME;

	g_deltaSeconds = (float) deltaSeconds;
	return (float) deltaSeconds;
}