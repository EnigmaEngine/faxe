import faxe.Faxe;

class Test
{
	static function main()
	{
		Faxe.fmod_init(36);

		// Load a sound bank
		Faxe.fmod_load_bank("./MasterBank.bank");

		// Make sure to load the STRINGS file to enable loading 
		// stuff by FMOD Path
		var result = Faxe.fmod_load_bank("./MasterBank.strings.bank");

    trace(result);

		// Load a test event
		Faxe.fmod_load_event("event:/FreakyMenu","toto");
		Faxe.fmod_play_event("event:/FreakyMenu");
    var state = Faxe.fmod_get_event_state("event:/FreakyMenu");
    trace(state);

		// Get and set an even parameter to change effect values
		trace("Lowpass param defaults to: " + Faxe.fmod_get_param("event:/testEvent", "Lowpass"));
		trace("Setting Lowpass param to 50.0");
		Faxe.fmod_set_param("event:/testEvent", "Lowpass", 50.0);

		// Bad little forever loop to pump FMOD commands
		while (true)
		{
			// trace("event:/testEvent is playing: " + Faxe.fmod_event_is_playing("event:/testEvent"));
			Faxe.fmod_update();
		}
	}
}
