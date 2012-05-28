#ifndef AQ_MOD_SELECTOR_H
#define AQ_MOD_SELECTOR_H

#include "AquariaMenuItem.h"
#include "DSQ.h"

class JuicyProgressBar : public Quad
{
public:
	JuicyProgressBar();

	void progress(float p);
	void showText(bool b);
	void setText(const std::string& s);
	inline float progress() { return perc; }
	BitmapText txt;
	Quad juice;

protected:
	float perc;
};

class BasicIcon : public AquariaGuiQuad
{
public:
	BasicIcon();
	std::string label;
	virtual bool isGuiVisible();

protected:
	bool mouseDown;
	Vector scaleNormal;
	Vector scaleBig;
	virtual void onUpdate(float dt);
	virtual void onClick();
};

class SubtitleIcon : public BasicIcon
{
protected:
	virtual void onUpdate(float dt);
};

class ModIcon : public SubtitleIcon
{
public:
	ModIcon();
	void loadEntry(const ModEntry& entry);
	virtual void updateStatus();
	std::string fname; // internal mod name (file/folder name)
	std::string modname; // mod name as given by author
	unsigned int modId;
	ModType modType;

protected:
	virtual void onClick();
};

class MenuIcon : public SubtitleIcon
{
public:
	typedef void (*callback)(int, void*);
	MenuIcon(int id);

	callback cb;
	void *cb_data;

protected:
	int iconId;
	virtual void onClick();
};

#ifdef BBGE_BUILD_VFS

class ModIconOnline : public SubtitleIcon
{
public:
	ModIconOnline();
	bool fixIcon();
	std::string namestr; // name of the mod: <Fullname text="..."/>
	std::string desc; // <Description text="..."/>
	std::string iconfile; // expected local texture file name
	std::string packageUrl; // where to download: <Package url="..." />

	std::string localname; // _mods/<localname>.aqmod - under which name the file should be stored if downloaded. Can be empty.
	std::string confirmStr; // <Confirm text="" /> -- pops up confirmation dialog before download if not empty.
	void setDownloadProgress(float p, float barheight = 20);
	virtual void updateStatus();
	JuicyProgressBar *pb; // visible if downloading
	Quad *extraIcon; // TODO: little icon in the lower right corner indicating status (out of date, newly added, has update, whatever)
	bool clickable;

protected:
	virtual void onClick();
};

#endif // BBGE_BUILD_VFS


class MenuBasicBar : public AquariaGuiQuad
{
public:
	MenuBasicBar();
	virtual void init();
};

class MenuIconBar : public MenuBasicBar
{
public:
	virtual void init();
	std::vector<MenuIcon*> icons;

protected:
	void add(MenuIcon *ico);
};

class MenuArrowBar : public MenuBasicBar
{
public:
	virtual void init();
};


class IconGridPanel : public AquariaGuiQuad
{
public:
	IconGridPanel();
	void fade(bool in, bool sc);
	void add(RenderObject *obj);
	int spacing;
	int getUsedX() const { return x; }
	int getUsedY() const { return y; }

protected:
	int x, y;
};

class ModSelectorScreen : public Quad, public ActionMapper
{
public:
	ModSelectorScreen();

	void init();
	void close();

	void showPanel(int id);
	void updateFade();
	
	void initModAndPatchPanel();
	void initNetPanel();

	void moveUp();
	void moveDown();
	void move(int ud);

	std::vector<IconGridPanel*> panels;

protected:
	virtual void onUpdate(float dt);
	MenuIconBar leftbar;
	MenuArrowBar rightbar;
	AquariaMenuItem arrowUp, arrowDown;
	int currentPanel;
	bool gotServerList;
};

#endif
