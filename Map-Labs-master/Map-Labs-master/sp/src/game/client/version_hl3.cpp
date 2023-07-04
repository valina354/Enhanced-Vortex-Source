// Include the header file for the engine interface
#include "engine/iserverplugin.h"
#include "convar.h"
#include "cbase.h"

// Define a global pointer to the engine interface
IServerPluginHelpers *helpers = NULL;

// Define a global variable for the message text
char *messageText = "Alpha 1.07 HL3 Citadel Unleashed";

// Define a global variable for the message color
Color messageColor = Color(255, 0, 0, 255); // Red

// Define a console variable to enable or disable the message
ConVar messageEnable("message_enable", "1", FCVAR_NONE, "Enable or disable the message");

// Define a console variable to change the message text
ConVar messageText("message_text", "Alpha 1.07 HL3 Citadel Unleashed", FCVAR_NONE, "Change the message text");

// Define a function to display the message on the top left corner of the screen
void DisplayMessage(edict_t *pEntity)
{
	// Get the player's user ID
	int userID = helpers->IndexOfEdict(pEntity);

	// Check if the message is enabled
	if (messageEnable.GetBool())
	{
		// Get the current message text from the console variable
		messageText = messageText.GetString();

		// Create a key-value pair for the message parameters
		KeyValues *kv = new KeyValues("msg");

		// Set the message type to text
		kv->SetString("type", "text");

		// Set the message position to top left corner
		kv->SetString("pos", "0.01 0.01");

		// Set the message text
		kv->SetString("msg", messageText);

		// Set the message color
		kv->SetColor("color", messageColor);

		// Set the message duration to 1 second
		kv->SetFloat("duration", 1.0f);

		// Send the message to the player using the engine interface
		helpers->CreateMessage(pEntity, DIALOG_MSG, kv, this);

		// Delete the key-value pair
		kv->deleteThis();
	}
}

// Define a function to hook into the game events
void GameFrame(bool simulating)
{
	// Loop through all the players in the game
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		// Get the entity of the current player
		edict_t *pEntity = engine->PEntityOfEntIndex(i);

		// Check if the entity is valid and connected
		if (pEntity && !pEntity->IsFree() && engine->GetPlayerNetInfo(i))
		{
			// Call the function to display the message on the top left corner of the screen
			DisplayMessage(pEntity);
		}
	}
}
