#pragma once

#include <string>

static const std::string GRASS_STR  ( "GRASS" );
static const std::string BLOCK_STR  ( "BLOCK" );
static const std::string START_STR  ( "START" );
static const std::string EXIT_STR   ( "EXIT" );
static const std::string WATER_STR  ( "WATER" );
static const std::string ROAD_STR   ( "ROAD" );
static const std::string FOREST_STR ( "FOREST" );
static const std::string HILL_STR   ( "MOUNTAIN" );
static const std::string VISITED_STR( "VISITED" );
static const std::string PATH_STR   ( "PATH" );

static const size_t EXIT_PATH_VALUE  = 0;
static const size_t ROAD_PATH_VALUE  = 1;
static const size_t GRASS_PATH_VALUE = 4;
static const size_t FOREST_PATH_VALUE= 12;
static const size_t HILL_PATH_VALUE  = 24;

static const double MAX_WEIGHT = 1e12;
