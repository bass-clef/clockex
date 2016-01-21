#pragma once

// appmain�Ŏg���N���X�܂Ƃ�

#include <string>

class modules;
class tasklist;
class app;
class form;
class chrono;
class dll;
template<class> class canvas;
template<class, class> class pen;
template<class, class> class image;
struct appinfo
{
	enum RUN_TIMING timing;	// �O��̃^�C�~���O

	app* appClass;					// appmain�S��
	form* windowInfo;				// �E�B���h�E���
	canvas<form>* clientInfo;		// �N���C�A���g���

	chrono* chrono;					// ���v
	pen<form, std::string>* pens;	// �y���Ǘ�
	image<form, std::string>* imgs;	// �摜�Ǘ�
	modules* tooltips;				// �c�[���`�b�v�Ǘ�
	tasklist* exque;				// ���s�^�C�~���O�Ǘ�
	dll* dlls;						// DLL�Ǘ�
};
