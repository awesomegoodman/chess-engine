// ============================================================
//  ~/chess/main.cpp  —  SFML 3.x
//
//  Controls:
//    Left-click a piece  → select it (highlighted)
//    Left-click a square → move the selected piece there
//    Right-click         → cancel selection
// ============================================================

#include <SFML/Graphics.hpp>

#include <array>
#include <cctype>
#include <optional>
#include <string>
#include <vector>

// ─── Visual constants

constexpr int SQUARE_PX = 80;
constexpr int MARGIN    = 40;
constexpr int BOARD_PX  = SQUARE_PX * 8;
constexpr int WINDOW_W  = BOARD_PX + MARGIN * 2;
constexpr int WINDOW_H  = BOARD_PX + MARGIN * 2;

const sf::Color COL_LIGHT     = {240, 217, 181};
const sf::Color COL_DARK      = {181, 136,  99};
const sf::Color COL_HIGHLIGHT = {130, 151,  99, 200};
const sf::Color COL_BG        = { 32,  32,  32};
const sf::Color COL_LABEL     = {160, 140, 120};

// ─── Minimal display board
//
//  '.' = empty
//  Uppercase = white piece,  lowercase = black piece
//  K/k King  Q/q Queen  R/r Rook  B/b Bishop  N/n Knight  P/p Pawn
//
//  Row 0 = rank 8 (black's back rank), row 7 = rank 1 (white's back rank).

using Board = std::array<std::array<char, 8>, 8>;

Board makeStartPosition()
{
    Board b{};
    b[0] = {'r','n','b','q','k','b','n','r'};
    b[1] = {'p','p','p','p','p','p','p','p'};
    for (int r = 2; r <= 5; ++r) b[r].fill('.');
    b[6] = {'P','P','P','P','P','P','P','P'};
    b[7] = {'R','N','B','Q','K','B','N','R'};
    return b;
}

// ─── Coordinate helpers

sf::Vector2f squareTopLeft(int col, int row)
{
    return { static_cast<float>(MARGIN + col * SQUARE_PX),
             static_cast<float>(MARGIN + row * SQUARE_PX) };
}

std::optional<sf::Vector2i> screenToSquare(sf::Vector2i mouse)
{
    if (mouse.x < MARGIN || mouse.y < MARGIN) return std::nullopt;
    int col = (mouse.x - MARGIN) / SQUARE_PX;
    int row = (mouse.y - MARGIN) / SQUARE_PX;
    if (col >= 8 || row >= 8) return std::nullopt;
    return sf::Vector2i{col, row};
}

// ─── Font loading

bool loadFont(sf::Font& font)
{
    static const std::vector<std::string> candidates = {
        // macOS
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Arial.ttf",
        "/Library/Fonts/Arial.ttf",
        // Linux
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        // Windows
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
    };
    for (const auto& path : candidates)
        if (font.openFromFile(path)) return true;
    return false;
}

// ─── Drawing

void drawSquares(sf::RenderWindow& win, std::optional<sf::Vector2i> selected)
{
    for (int row = 0; row < 8; ++row)
    {
        for (int col = 0; col < 8; ++col)
        {
            sf::RectangleShape sq(sf::Vector2f{(float)SQUARE_PX, (float)SQUARE_PX});
            sq.setPosition(squareTopLeft(col, row));
            sq.setFillColor((row + col) % 2 == 0 ? COL_LIGHT : COL_DARK);
            win.draw(sq);

            if (selected && selected->x == col && selected->y == row)
            {
                sq.setFillColor(COL_HIGHLIGHT);
                win.draw(sq);
            }
        }
    }
}

void drawLabels(sf::RenderWindow& win, const sf::Font& font)
{
    for (int i = 0; i < 8; ++i)
    {
        sf::Text rank(font, std::to_string(8 - i), 13);
        rank.setFillColor(COL_LABEL);
        auto rb = rank.getLocalBounds();
        rank.setPosition(sf::Vector2f{
            (float)(MARGIN - 22),
            (float)(MARGIN + i * SQUARE_PX + SQUARE_PX / 2) - rb.size.y / 2.f - rb.position.y
        });
        win.draw(rank);

        sf::Text file(font, std::string(1, (char)('a' + i)), 13);
        file.setFillColor(COL_LABEL);
        auto fb = file.getLocalBounds();
        file.setPosition(sf::Vector2f{
            (float)(MARGIN + i * SQUARE_PX + SQUARE_PX / 2) - fb.size.x / 2.f - fb.position.x,
            (float)(MARGIN + BOARD_PX + 8)
        });
        win.draw(file);
    }
}

void drawPieces(sf::RenderWindow& win, const Board& board, const sf::Font& font)
{
    constexpr float RADIUS  = SQUARE_PX * 0.37f;
    constexpr float OUTLINE = 2.0f;

    for (int row = 0; row < 8; ++row)
    {
        for (int col = 0; col < 8; ++col)
        {
            char pc = board[row][col];
            if (pc == '.') continue;

            const bool isWhite = (std::isupper(pc) != 0);
            auto [ox, oy] = squareTopLeft(col, row);
            float cx = ox + SQUARE_PX / 2.f;
            float cy = oy + SQUARE_PX / 2.f;

            sf::CircleShape circle(RADIUS);
            circle.setOrigin(sf::Vector2f{RADIUS, RADIUS});
            circle.setPosition(sf::Vector2f{cx, cy});
            circle.setFillColor(isWhite ? sf::Color(248, 248, 240)
                                        : sf::Color( 40,  40,  40));
            circle.setOutlineThickness(OUTLINE);
            circle.setOutlineColor(isWhite ? sf::Color(160, 160, 150)
                                           : sf::Color( 90,  90,  90));
            win.draw(circle);

            sf::Text letter(font, std::string(1, (char)std::toupper(pc)), 26);
            letter.setStyle(sf::Text::Bold);
            letter.setFillColor(isWhite ? sf::Color( 40,  40,  40)
                                        : sf::Color(215, 215, 210));
            auto lb = letter.getLocalBounds();
            letter.setOrigin(sf::Vector2f{
                lb.position.x + lb.size.x / 2.f,
                lb.position.y + lb.size.y / 2.f
            });
            letter.setPosition(sf::Vector2f{cx, cy});
            win.draw(letter);
        }
    }
}

// ─── Main

int main()
{
    sf::RenderWindow window(
        sf::VideoMode({(unsigned)WINDOW_W, (unsigned)WINDOW_H}),
        "Chess Board");
    window.setFramerateLimit(60);

    sf::Font font;
    loadFont(font);

    Board board = makeStartPosition();
    std::optional<sf::Vector2i> selected;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* mp = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mp->button == sf::Mouse::Button::Left)
                {
                    auto sq = screenToSquare(mp->position);
                    if (!sq)
                    {
                        selected = std::nullopt;
                    }
                    else if (!selected)
                    {
                        if (board[sq->y][sq->x] != '.')
                            selected = sq;
                    }
                    else
                    {
                        board[sq->y][sq->x]             = board[selected->y][selected->x];
                        board[selected->y][selected->x] = '.';
                        selected = std::nullopt;
                    }
                }

                if (mp->button == sf::Mouse::Button::Right)
                    selected = std::nullopt;
            }
        }

        window.clear(COL_BG);
        drawSquares(window, selected);
        drawLabels (window, font);
        drawPieces (window, board, font);
        window.display();
    }

    return 0;
}