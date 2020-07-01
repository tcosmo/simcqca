#pragma once

#include <assert.h>
#include <set>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>

#endif

#define pritnf printf // My dyslexia

#define MIN(X,Y) ((X < Y) ? X : Y)
#define MAX(X,Y) ((X < Y) ? Y : X)


#include <SFML/Graphics.hpp> // for sf::Vector2i

#define EAST sf::Vector2i({1, 0})
#define WEST sf::Vector2i({-1, 0})
#define SOUTH sf::Vector2i({0, 1})
#define NORTH sf::Vector2i({0, -1})

struct compareWorldPositions {
  bool operator()(const sf::Vector2<int> &a, const sf::Vector2<int> &b) const {
    if (a.x == b.x)
      return a.y < b.y;
    return a.x < b.x;
  }
};

static sf::Vector2i operator*(int scalar, const sf::Vector2i &vector) {
  sf::Vector2i toReturn = vector;
  for (int i = 0; i < scalar; i += 1)
    toReturn += vector;
  return toReturn;
}

typedef std::set<sf::Vector2i, compareWorldPositions> Poset;

static void saveFile(const std::string &filePath,
                     const std::string &fileContent) {
  FILE *f = fopen(filePath.c_str(), "w");
  if (f == NULL) {
    printf("Could not save content to `%s`.\n", filePath.c_str());
    return;
  }
  fprintf(f, "%s", fileContent.c_str());
  fclose(f);
  printf("Written content to `%s`.\n", filePath.c_str());
  return;
}