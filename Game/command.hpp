#pragma once

#include <SDL3/SDL.h>

class Command {
public:
	virtual void ~Command;
	virtual void execute() = 0;
};

class MoveLeft : public Command {
public:
	virtual void execute() { moveLeft(); }
};

class MoveRight : public Command {
public:
	virtual void execute() { moveRight(); }
};

class MoveUp : public Command {
public:
	virtual void execute() { moveUp(); }
};


class MoveDown : public Command {
public:
	virtual void execute() { moveDown(); }
};

class Select : public Command {
public:
	virtual void execute() { select(); }
};
