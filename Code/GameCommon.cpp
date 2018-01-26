#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
float g_deltaSeconds = 0.f;
bool g_isSavingAndLoading = false;
bool g_loadAllChunksOnStartup = true;
bool g_isWeatherActive = false;
bool g_isHelpActive = false;