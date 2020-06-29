#include "../graphic_engine.h"

void GraphicEngine::cameraTranslate(float dx, float dy) {
  camera.move(dx, dy);
  window.setView(camera);
}

void GraphicEngine::cameraTranslate(const sf::Vector2f& vec) {
  cameraTranslate(vec.x, vec.y);
}

void GraphicEngine::cameraZoom(float zoom_factor) {
  currentZoom *= zoom_factor;
  camera.zoom(1 / zoom_factor);
  window.setView(camera);
}

void GraphicEngine::cameraCenter(const sf::Vector2f& where) {
  camera.setCenter(where);
  window.setView(camera);
}

void GraphicEngine::handleCameraEvents(const sf::Event& event) {
  // Translate view with CTRL + Arrows and center when press C
  if (event.type == sf::Event::KeyPressed) {
    switch (event.key.code) {
      case sf::Keyboard::C:
        cameraCenter({0, 0});
        break;

      case sf::Keyboard::A:
        if (event.key.control) cameraZoom(DEFAULT_CAM_ZOOM_STEP);
        break;

      case sf::Keyboard::Z:
        if (event.key.control) cameraZoom(1 / DEFAULT_CAM_ZOOM_STEP);
        break;

      case sf::Keyboard::Up:
        if (event.key.control && isShiftPressed())
          cameraTranslate(0, -DEFAULT_CAM_TRANSLATION * CELL_H);
        break;

      case sf::Keyboard::Down:
        if (event.key.control && isShiftPressed())
          cameraTranslate(0, DEFAULT_CAM_TRANSLATION * CELL_H);
        break;

      case sf::Keyboard::Right:
        if (event.key.control && isShiftPressed())
          cameraTranslate(DEFAULT_CAM_TRANSLATION * CELL_W, 0);
        break;

      case sf::Keyboard::Left:
        if (event.key.control && isShiftPressed())
          cameraTranslate(-DEFAULT_CAM_TRANSLATION * CELL_W, 0);
        break;

      default:
        break;
    }
  }

  // Track mouse left events for replacing mouse in center if left
  if (event.type == sf::Event::MouseLeft) cameraMouseLeft = true;
  if (event.type == sf::Event::MouseEntered) cameraMouseLeft = false;

  if (event.type == sf::Event::MouseWheelScrolled)
    if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
      if (isControlPressed()) {
        float zoomFactor = DEFAULT_CAM_ZOOM_STEP;
        if (event.mouseWheelScroll.delta < 0) zoomFactor = 1 / zoomFactor;
        cameraZoom(zoomFactor);
      }
    }

  if (event.type == sf::Event::MouseButtonPressed)
    if ((event.mouseButton.button == sf::Mouse::Middle)) {
      moveCameraMode = true;
      cameraMousePosition = sf::Mouse::getPosition(window);
    }

  if (event.type == sf::Event::MouseMoved) {
    if (moveCameraMode) {
      auto coordMouseMove =
          window.mapPixelToCoords({event.mouseMove.x, event.mouseMove.y});
      auto coordMouse = window.mapPixelToCoords(cameraMousePosition);
      cameraTranslate(coordMouse - coordMouseMove);
      cameraMousePosition = sf::Mouse::getPosition(window);
    }
  }

  if (event.type == sf::Event::MouseButtonReleased)
    if (event.mouseButton.button == sf::Mouse::Middle) {
      moveCameraMode = false;

      if (cameraMouseLeft)
        sf::Mouse::setPosition({static_cast<int>(window.getSize().x / 2),
                                static_cast<int>(window.getSize().y / 2)},
                               window);
    }
}

std::pair<sf::Vector2i, sf::Vector2i>
GraphicEngine::getExtremalVisibleCellsPos() {
  /***
   * Returns (topLeft,bottomRight) in world positions.
   */

  sf::Vector2f topLeftCoords = window.mapPixelToCoords({0, 0});
  sf::Vector2i topLeftPos = mapCoordsToWorldPos(topLeftCoords);

  sf::Vector2f bottomRightCoords = window.mapPixelToCoords(
      {(int)window.getSize().x, (int)window.getSize().y});
  sf::Vector2i bottomRightPos = mapCoordsToWorldPos(bottomRightCoords);

  return std::make_pair(topLeftPos, bottomRightPos);
}

bool GraphicEngine::isCellInView(const sf::Vector2i& cellPos) {
  auto extremalVisibleCells = getExtremalVisibleCellsPos();
  auto topLeft = extremalVisibleCells.first;
  auto bottomRight = extremalVisibleCells.second;

  return cellPos.x >= topLeft.x && cellPos.x <= bottomRight.x &&
         cellPos.y >= topLeft.y && cellPos.y <= bottomRight.y;
}