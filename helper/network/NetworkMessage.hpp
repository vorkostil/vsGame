#pragma once

#include <string>

static const std::string MESSAGE_INIT( "SYSTEM_INIT_CONNECTION" );
static const std::string MESSAGE_LOGIN_ASKED( "SYSTEM_LOGIN_ASKED" );
static const std::string MESSAGE_LOGIN_ACCEPTED( "SYSTEM_LOGIN_ACCEPTED" );
static const std::string MESSAGE_LOGIN_REFUSED( "SYSTEM_LOGIN_REFUSED" );
static const std::string MESSAGE_CLOSE( "SYSTEM_CLOSE_CONNECTION" );

static const std::string SYSTEM_REGISTER( "SYSTEM_REGISTER" );
static const std::string SYSTEM_REQUEST_GAME( "SYSTEM_REQUEST_GAME" );
static const std::string SYSTEM_JOIN_GAME( "SYSTEM_JOIN_GAME" );
static const std::string SYSTEM_LEAVE_GAME( "SYSTEM_LEAVE_GAME" );

static const std::string CONSUMER_PART( "CONSUMER" );
static const std::string PROVIDER_PART( "PROVIDER" );

static const std::string SYSTEM_GAME_REFUSED( "SYSTEM_GAME_REFUSED" );
static const std::string SYSTEM_GAME_ACCEPTED( "SYSTEM_GAME_ACCEPTED" );

static const std::string SYSTEM_JOIN_GAME_REFUSED( "SYSTEM_JOIN_GAME_REFUSED" );

static const std::string GAME_CLOSE_MESSAGE( "GAME_CLOSE" );
static const std::string GAME_MESSAGE( "GAME_MESSAGE" );

#define CREATE_MESSAGE( command, message ) command + " " + message
