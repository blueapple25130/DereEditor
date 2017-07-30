#pragma once
#include "Note.h"


class PlayableNote :public Note {
private:
	NoteType m_type;

public:
	std::shared_ptr<PlayableNote> PlevNote = nullptr;
	std::shared_ptr<PlayableNote> NextNote = nullptr;
	int Channel;
	int StartPos;
	int FinishPos;
	bool IsJudged = false;

	PlayableNote(int tick, int startPos, int finishPos, int channel, NoteType type);

	PlayableNote() = default;
	~PlayableNote() = default;

	virtual void draw() const;

	void drawRibbon() const;

	void drawInfo() const;

	NoteType getNoteType();

	void setStartPos(int lane);
};