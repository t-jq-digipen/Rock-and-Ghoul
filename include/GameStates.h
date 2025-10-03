// Test file, maybe delete?
#pragma once

class GSM; //fwd declaration

class IGameState {
protected:
    GSM& gsm;
public:
    IGameState(GSM& gsmref) : gsm(gsmref) {}
    ~IGameState() = default;

	// virtual classes to be overridden
	// Loads main menu
	virtual void Load()		= 0;
	// Initializes main menu
	virtual void Init()		= 0;
	// Updates main menu
	virtual void Update()	= 0;
	// Draws main menu
	virtual void Draw()		= 0;
	// Frees main menu
	virtual void Free()		= 0;
	// Unloads main menu
	virtual void Unload()	= 0;
};

class newMainMenu : public IGameState
{
public:
	newMainMenu(GSM& gsmref) : IGameState(gsmref) {}

	// Loads main menu
	void Load()		override;
	// Initializes main menu
	void Init()		override;
	// Updates main menu
	void Update()	override;
	// Draws main menu
	void Draw()		override;
	// Frees main menu
	void Free()		override;
	// Unloads main menu
	void Unload()	override;
};

class newLevel1 : public IGameState
{
	// Loads
	void Load()		override;
	// Initializes
	void Init()		override;
	// Updates
	void Update()	override;
	// Draws
	void Draw()		override;
	// Frees
	void Free()		override;
	// Unloads
	void Unload()	override;
};