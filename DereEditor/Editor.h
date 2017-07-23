#pragma once
#include "Measure.h"
#include "Note.h"
#include "PlayableNote.h"
#include "ChangeTempo.h"

class Editor {
private:
	std::vector<std::shared_ptr<Measure>> m_measures;
	std::vector<std::shared_ptr<Note>> m_notes;
	const int BeatPerHeight = 100;
	const int m_LaneOffset = 125;
	const int m_LaneWidth = 70;
	const int m_division = 8;
	double m_zoom = 1.0;
	const Rect m_rect;
	int m_selectedMeasure = 1;
	GUI m_gui;
	Rect m_selectedRect;
	Point m_selectBegin;
	Point m_selectEnd;

	/*
	ノート配置の更新がある時、必ず呼ぶ
	*/
	void updateNoteState();

	double getBeatPerHeight();

	int getLaneOffset();

	void setOrRemoveNote(std::vector<std::shared_ptr<Note>> &notes, int tick, int lane);

public:

	Editor();
	~Editor() = default;

	void update();
	void draw();
};