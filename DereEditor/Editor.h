#pragma once
#include "Measure.h"
#include "Note.h"
#include "PlayableNote.h"
#include "ChangeTempo.h"

class Editor {
private:
	std::vector<std::shared_ptr<Measure>> m_measures;
	std::vector<std::shared_ptr<Note>> m_notes;
	std::vector<std::shared_ptr<PlayableNote>> m_playableNotes;
	const int BeatPerHeight = 100;
	const int m_LaneOffset = 125;
	const int m_LaneWidth = 70;
	int m_division = 8;
	int m_channel = 0;
	const int m_offset = 60;
	const size_t MEASURE_MAX = 999;
	const size_t MEASURE_MIN = 1;
	const size_t MEASURE_DEFAULT = 5;
	double m_zoom = 1.0;
	const Rect m_rect;
	size_t m_selectedMeasure = 1;
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

	void setOrRemoveNote(std::vector<std::shared_ptr<Note>> &notes, int tick, int lane, NoteType type);

	void changeZoomRatio();

	void channelUp();
	void channelDown();

	void addMeasure();
	void removeMeasure();
	void setMeasureLength();


public:

	Editor();
	~Editor() = default;

	void update();
	void draw();
};