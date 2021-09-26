/**
* Faxe - FMOD bindings for Haxe
*
* The MIT License (MIT)
*
* Copyright (c) 2016 Aaron M. Shea
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include <hxcpp.h>
#include <fmod_studio.hpp>
#include <fmod_errors.h>
#include <map>

#include "linc_faxe.h"

/**
 * Update to 2.x requires minor refactors.
 * @see https://fmod.com/resources/documentation-api?version=2.02&page=welcome-whats-new-200.html
 */
namespace linc 
{
	namespace faxe
	{
		// FMOD Sound System
		FMOD::Studio::System* fmodSoundSystem;
		FMOD::System* fmodLowLevelSoundSystem;

		// Maps to track what has been loaded already
		std::map<::String, FMOD::Studio::Bank*> loadedBanks;
		std::map<::String, FMOD::Sound*> loadedSounds;
		std::map<::String, FMOD::Studio::EventInstance*> loadedEvents;
		
		bool faxe_debug = false;
		void faxe_set_debug(bool onOff){
			faxe_debug = onOff;
		}
		
		FMOD::System* faxe_get_system(){
			return fmodLowLevelSoundSystem;
		}

		//// FMOD Init
		void faxe_init(int numChannels)
		{
			// Create our new fmod system
			if (FMOD::Studio::System::create(&fmodSoundSystem) != FMOD_OK)
			{
				if(faxe_debug) printf("Failure starting FMOD sound system!");
				return;
			}

			// All OK - Setup some channels to work with!
			fmodSoundSystem->initialize(numChannels, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
			// ERIC: The LowLevel API has been renamed to Core API, other than that it should still function the same way it previously has.
			fmodSoundSystem->getCoreSystem(&fmodLowLevelSoundSystem);
			if(faxe_debug) printf("FMOD Sound System Started with %d channels!\n", numChannels);
		}

		void faxe_update()
		{
			fmodSoundSystem->update();
		}

		//// Sound Banks
		bool faxe_load_bank(const ::String& bankName)
		{
			// Ensure this isn't already loaded
			if (loadedBanks.find(bankName) != loadedBanks.end())
			{
				return true;
			}

			// Try and load the bank file
			FMOD::Studio::Bank* tempBank;
			auto result = fmodSoundSystem->loadBankFile(bankName.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &tempBank);
			if (result != FMOD_OK)
			{
				if(faxe_debug) printf("FMOD failed to LOAD sound bank %s with error %s\n", bankName.c_str(), FMOD_ErrorString(result));
				return false;
			}

			// List is as loaded
			loadedBanks[bankName] = tempBank;
			return true;
		}

		void faxe_unload_bank(const ::String& bankName)
		{
			// Ensure this bank exists
			auto found = loadedBanks.find(bankName);
			if (found != loadedBanks.end())
			{
				// Remove from loaded banks map
				loadedBanks.erase(bankName);

				// Unload the bank that matches
				found->second->unload();
			}
		}

		FMOD::Sound* faxe_get_sound(const ::String& sndName) {
			if (loadedSounds.find(sndName) == loadedSounds.end()){
				if(faxe_debug) printf("not loaded \n");
				return nullptr;
			}
			return loadedSounds[sndName];
		}
		
		
		
		FMOD_RESULT faxe_load_sound(const ::String& sndName, bool looping, bool streaming)
		{
			// Ensure the sound has not already been loaded
			if (loadedSounds.find(sndName) != loadedSounds.end())
			{
				if(faxe_debug) printf("already loaded\n");
				return FMOD_OK;
			}

			FMOD_MODE loadSndMode = FMOD_DEFAULT;
			if (looping)		loadSndMode |= FMOD_LOOP_NORMAL;
			if (streaming)		loadSndMode |= FMOD_CREATESTREAM;

			// Try and load this sound
			FMOD::Sound* tempSound;
			auto result = fmodLowLevelSoundSystem->createSound(sndName.c_str(), loadSndMode, nullptr, &tempSound);
			if (result != FMOD_OK)
			{
				if(faxe_debug) printf("FMOD failed to LOAD sound %s with error %s\n", sndName.c_str(), FMOD_ErrorString(result));
				return result;
			}

			// Store in loaded sounds map
			loadedSounds[sndName] = tempSound;
			return result;
		}
		
		FMOD_RESULT faxe_play_sound_with_handle( FMOD::Sound * snd)
		{
			FMOD_RESULT res = fmodLowLevelSoundSystem->playSound(snd, nullptr, false, nullptr);
			if(faxe_debug && res ) printf("error playing\n");
			return res;
		}
		
		FMOD_RESULT faxe_play_sound(const ::String& sndName, bool paused)
		{
			if (loadedSounds.find(sndName) == loadedSounds.end())
			{
				if(faxe_debug) printf("not loaded \n");
				return FMOD_ERR_INVALID_PARAM;
			}
			
			FMOD::Sound* snd = loadedSounds[sndName];
			FMOD_RESULT res = fmodLowLevelSoundSystem->playSound(snd, nullptr, paused, nullptr);
			if(faxe_debug && res ) printf("error playing\n");
			return res;
		}
		
		FMOD::Channel * faxe_play_sound_with_channel(const ::String& sndName, bool paused)
		{
			if (loadedSounds.find(sndName) == loadedSounds.end())
			{
				if(faxe_debug) printf("not loaded \n");
				return nullptr;
			}
			
			FMOD::Sound* snd = loadedSounds[sndName];
			FMOD::Channel * chan = nullptr;
			int res = fmodLowLevelSoundSystem->playSound(snd, nullptr, paused, &chan);
			return chan;
		}

		void faxe_unload_sound(const ::String& sndName)
		{
			auto found = loadedSounds.find(sndName);

			// Ensure the sound has already been loaded
			if (found != loadedSounds.end())
			{
				// Remove from loaded map
				loadedSounds.erase(sndName);

				// Unload the sound
				found->second->release();
			}
		}

		void faxe_load_event(const ::String& eventPath, const ::String& eventName)
		{
			// Check it's not already loaded
			if (loadedEvents.find(eventName) != loadedEvents.end())
			{
				return;
			}

			// Try and load this event description
			FMOD::Studio::EventDescription* tempEvnDesc;

			auto result = fmodSoundSystem->getEvent(eventPath.c_str(), &tempEvnDesc);

			if (result != FMOD_OK)
			{
				if(faxe_debug) printf("FMOD failed to LOAD event instance %s with error %s\n", eventPath.c_str(), FMOD_ErrorString(result));
				return;
			}

			// Now create an instance of this event that we can keep in memory
			FMOD::Studio::EventInstance* tempEvnInst;
			result = tempEvnDesc->createInstance(&tempEvnInst);

			if (result != FMOD_OK)
			{
				if(faxe_debug) printf("FMOD failed to CREATE INSTANCE of event instance %s with error %s\n", eventPath.c_str(), FMOD_ErrorString(result));
				return;
			}

			// Store in event map
			loadedEvents[eventName] = tempEvnInst;
		}

		void faxe_play_event(const ::String& eventName)
		{
			// Ensure that the event is loaded first
			auto targetEvent = loadedEvents.find(eventName);
			if (targetEvent != loadedEvents.end())
			{
				// Start the event instance
				targetEvent->second->start();
			} else {
				if(faxe_debug) printf("Event %s is not loaded!\n", eventName.c_str());
			}
		}

		void faxe_stop_event(const ::String& eventName, bool forceStop)
		{
			// Find the event first
			auto targetStopEvent = loadedEvents.find(eventName);
			if (targetStopEvent != loadedEvents.end())
			{
				FMOD_STUDIO_STOP_MODE stopMode;

				if (forceStop)
				{
					stopMode = FMOD_STUDIO_STOP_IMMEDIATE;
				} else {
					stopMode = FMOD_STUDIO_STOP_ALLOWFADEOUT;
				}

				// Stop the event
				targetStopEvent->second->stop(stopMode);

			} else {
				if(faxe_debug) printf("Event %s is not loaded!\n", eventName.c_str());
			}
		}

		bool faxe_event_playing(const ::String& eventName)
		{
			auto targetEvent = loadedEvents.find(eventName);
			if (targetEvent != loadedEvents.end())
			{
				// Check the playback state of this event
				FMOD_STUDIO_PLAYBACK_STATE currentState;
				auto result = targetEvent->second->getPlaybackState(&currentState);

				if (result != FMOD_OK)
				{
					if(faxe_debug) printf("FMOD failed to GET PLAYBACK STATUS of event instance %s with error %s\n", eventName.c_str(), FMOD_ErrorString(result));
					return false;
				}

				return (currentState == FMOD_STUDIO_PLAYBACK_PLAYING);
			} else {
				if(faxe_debug) printf("Event %s is not loaded!\n", eventName.c_str());
				return false;
			}
		}

		FMOD_STUDIO_PLAYBACK_STATE faxe_get_event_state(const ::String& eventName)
		{
			auto targetEvent = loadedEvents.find(eventName);
			if (targetEvent != loadedEvents.end())
			{
				// Check the playback state of this event
				FMOD_STUDIO_PLAYBACK_STATE currentState;
				auto result = targetEvent->second->getPlaybackState(&currentState);

				if (result != FMOD_OK)
				{
					if(faxe_debug) printf("FMOD failed to GET PLAYBACK STATUS of event instance %s with error %s\n", eventName.c_str(), FMOD_ErrorString(result));
					return FMOD_STUDIO_PLAYBACK_STOPPED;
				}

				return currentState;
			} else {
				if(faxe_debug) printf("Event %s is not loaded!\n", eventName.c_str());
				return FMOD_STUDIO_PLAYBACK_STOPPED;
			}
		}

		float faxe_get_event_param(const ::String& eventName, const ::String& paramName)
		{
			auto targetEvent = loadedEvents.find(eventName);
			if (targetEvent != loadedEvents.end())
			{
				// Try and get the float param from EventInstance
				float currentValue;
				// Studio::EventInstance::getParameterValue is now Studio::EventInstance::getParameterByName
				auto result = targetEvent->second->getParameterByName(paramName.c_str(), &currentValue);

				if (result != FMOD_OK)
				{
					if(faxe_debug) printf("FMOD failed to GET PARAM %s of event instance %s with error %s\n", paramName.c_str(), eventName.c_str(), FMOD_ErrorString(result));
					return -1;
				}

				return currentValue;
			} else {
				if(faxe_debug) printf("Event %s is not loaded!\n", eventName.c_str());
				return -1;
			}
		}

		bool faxe_event_paused(const ::String& eventName)
		{
			auto targetEvent = loadedEvents.find(eventName);
			if (targetEvent != loadedEvents.end())
			{
				// Try and get the paused param from EventInstance
				bool currentValue;
				auto result = targetEvent->second->getPaused(&currentValue);

				if (result != FMOD_OK)
				{
					if(faxe_debug) printf("FMOD failed to GET pause status of event instance %s with error %s\n", eventName.c_str(), FMOD_ErrorString(result));
					return false;
				}

				return currentValue;
			} else {
				if(faxe_debug) printf("Event %s is not loaded!\n", eventName.c_str());
				return false;
			}
		}

		bool faxe_pause_event(const ::String& eventName, bool shouldPause)
		{
			auto targetEvent = loadedEvents.find(eventName);
			if (targetEvent != loadedEvents.end())
			{
				auto result = targetEvent->second->setPaused(shouldPause);

				if (result != FMOD_OK)
				{
					if(faxe_debug) printf("FMOD failed to SET pause status to %s of event instance %s with error %s\n", shouldPause ? "true" : "false", eventName.c_str(), FMOD_ErrorString(result));
					return false;
				}

				// Success.
				return true;
			} else {
				if(faxe_debug) printf("Event %s is not loaded!\n", eventName.c_str());
				return false;
			}
		}


		bool faxe_set_event_param(const ::String& eventName, const ::String& paramName, float sValue)
		{
			auto targetEvent = loadedEvents.find(eventName);
			if (targetEvent != loadedEvents.end())
			{
				// Studio::EventInstance::setParameterValue is now Studio::EventInstance::setParameterByName
				auto result = targetEvent->second->setParameterByName(paramName.c_str(), sValue);

				if (result != FMOD_OK)
				{
					if(faxe_debug) printf("FMOD failed to SET PARAM %s of event instance %s with error %s\n", paramName.c_str(), eventName.c_str(), FMOD_ErrorString(result));
					return false;
				}

				// Success.
				return true;
			} else {
				if(faxe_debug) printf("Event %s is not loaded!\n", eventName.c_str());
				return false;
			}
		}

	} // faxe + fmod namespace
} // linc namespace
