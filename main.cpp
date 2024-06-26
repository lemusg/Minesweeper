#include <fstream>
#include <map>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <random>
#include <ctime>
#include <chrono>
using namespace std;
using namespace sf;

void setText(Text &text, float x, float y){
    FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width/2.0f,textRect.top + textRect.height/2.0f);
    text.setPosition(Vector2f(x, y));
}
struct Tile {
    int row;
    int col;
    vector<Tile*> neighbors;
    bool hasMine;
    bool isRevealed;
    bool isFlagged;
    int numMinesAround;
    Sprite sprite;
    Tile(){
        row = 0;
        col = 0;
        hasMine = false;
        isRevealed = false;
        isFlagged = false;
        numMinesAround = 0;
    }
    Tile(int r, int c){
        row = r;
        col = c;
        hasMine = false;
        isRevealed = false;
        isFlagged = false;
        numMinesAround = 0;
    }
};
void setBoard(vector<vector<Tile>> &board, int mines, map<string, Texture> &textures) {
    for (int i = 0; i < board.size(); i++)
        for (int j = 0; j < (board[0]).size(); j++) {
            (board[i])[j].hasMine = false;
            (board[i])[j].isRevealed = false;
            (board[i])[j].isFlagged = false;
            (board[i])[j].numMinesAround = 0;
            (board[i])[j].neighbors.clear();
            (board[i])[j].sprite.setTexture(textures["hidden"]);
        }
    mt19937 rng(time(nullptr));
    uniform_int_distribution<int> dist(0, 99);
    int minesLeft = mines;
    while (minesLeft > 0)
        for (int i = 0; i < board.size(); i++)
            for (int j = 0; j < (board[0]).size(); j++) {
                int rand = dist(rng);
                if (rand == 50 && !(board[i])[j].hasMine && minesLeft > 0) {
                    (board[i])[j].hasMine = true;
                    minesLeft--;
                }
            }
    for (int i = 0; i < board.size(); i++)
        for (int j = 0; j < (board[0]).size(); j++) {
            if (i - 1 >= 0) {
                if (j - 1 >= 0) {
                    (board[i])[j].neighbors.push_back(&(board[i - 1])[j - 1]);
                }
                if (j + 1 < (board[0]).size()) {
                    (board[i])[j].neighbors.push_back(&(board[i - 1])[j + 1]);
                }
                (board[i])[j].neighbors.push_back(&(board[i - 1])[j]);
            }
            if (j - 1 >= 0)
                (board[i])[j].neighbors.push_back(&(board[i])[j - 1]);
            if (j + 1 < (board[0]).size())
                (board[i])[j].neighbors.push_back(&(board[i])[j + 1]);
            if (i + 1 < board.size()) {
                if (j - 1 >= 0)
                    (board[i])[j].neighbors.push_back(&(board[i + 1])[j - 1]);
                if (j + 1 < board[0].size())
                    (board[i])[j].neighbors.push_back(&(board[i + 1])[j + 1]);
                (board[i])[j].neighbors.push_back(&(board[i + 1])[j]);
            }
        }
    for (int i = 0; i < board.size(); i++)
        for (int j = 0; j < board[0].size(); j++)
            for (int k = 0; k < (board[i])[j].neighbors.size(); k++)
                if ((board[i])[j].neighbors[k]->hasMine)
                    (board[i])[j].numMinesAround++;
}
void checkTiles(Tile &tile, int &numRevealed) {
    numRevealed++;
    tile.isRevealed = true;
    for (auto & neighbor : tile.neighbors) {
        if (!(neighbor->hasMine) && !(neighbor->isRevealed) && !(neighbor->isFlagged)) {
            if (neighbor->numMinesAround == 0) {
                checkTiles(*neighbor, numRevealed);
            }
        }
        if (!(neighbor->hasMine) && !(neighbor->isRevealed) && neighbor->numMinesAround != 0 && !(neighbor->isFlagged)) {
            numRevealed++;
            neighbor->isRevealed = true;
        }
    }
}
void updateLeaders(int time, string &leaders, string name, bool gameOver) {
    ifstream leaderFile("files/leaderboard.txt");
    vector<string> leadersVect(5);
    getline(leaderFile, leadersVect[0]);
    getline(leaderFile, leadersVect[1]);
    getline(leaderFile, leadersVect[2]);
    getline(leaderFile, leadersVect[3]);
    getline(leaderFile, leadersVect[4]);
    if (gameOver) {
        for (int i = 0; i < 5; i++) {
            if ((stoi(leadersVect[i].substr(0, 2)) * 60) + (stoi(leadersVect[i].substr(3, 2))) > time) {
                int min = time / 60;
                int sec = time % 60;
                string newLeader;
                if (min < 10)
                    newLeader = "0" + to_string(min) + ":";
                else
                    newLeader = to_string(min) + ":";
                if (sec < 10)
                    newLeader += "0" + to_string(sec);
                else
                    newLeader += to_string(sec);
                newLeader += ", " + name + "*";
                leadersVect.insert(leadersVect.begin() + i, newLeader);
                leadersVect.pop_back();
                break;
            }
        }
        ofstream output("files/leaderboard.txt");
        for (string lead: leadersVect)
            output << lead << endl;
    }
    leaders = "";
    for (int i = 0; i < 5; i++) {
        leaders += to_string(i + 1) + ".\t" + leadersVect[i].substr(0, 5) + "\t" + leadersVect[i].substr(6, leadersVect[i].size() - 5);
        if (i < 4)
            leaders += "\n\n";
    }
}
int main() {
    ifstream file("files/config.cfg");
    string c;
    getline(file, c);
    string r;
    getline(file, r);
    string m;
    getline(file, m);
    int numRevealed = 0;
    int col = stoi(c);
    int row = stoi(r);
    int mines = stoi(m);
    int flags = mines;
    float width = (float) col * 32;
    float height = (float) row * 32 + 100;
    RenderWindow window(VideoMode((int)width,(int)
    height), "Minesweeper");
    Font font;
    font.loadFromFile("files/font.ttf");
    Text welcome("WELCOME TO MINESWEEPER!", font, 24);
    welcome.setFillColor(Color::White);
    welcome.setStyle(Text::Bold);
    welcome.setStyle(Text::Underlined);
    setText(welcome, width / 2, height / 2 - 150);
    Text welcome2("Enter your name:", font, 20);
    welcome2.setFillColor(Color::White);
    welcome2.setStyle(Text::Bold);
    setText(welcome2, width / 2, height / 2 - 75);
    string name = "|";
    Text _name(name, font, 18);
    _name.setFillColor(Color::Yellow);
    _name.setStyle(Text::Bold);
    setText(_name, width / 2, height / 2 - 45);
    bool cont = true;
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
                cont = false;
            }
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::BackSpace && name.size() > 1) {
                    if (name[name.size() - 1] == '|')
                        name = name.substr(0, name.size() - 2) + "|";
                    else
                        name = name.substr(0, name.size() - 1) + "|";
                    _name.setString(name);
                    setText(_name, width / 2, height / 2 - 45);
                }
                if (event.key.code == Keyboard::Enter && name.size() != 1) {
                    if (name[name.size() - 1] == '|' && name.size() > 1) {
                        name = name.substr(0, name.size() - 1);
                        window.close();
                    }
                }
            }
            if (event.type == Event::TextEntered)
                if (isalpha(event.text.unicode)) {
                    char ch = static_cast<char>(event.text.unicode);
                    if (name.size() == 1)
                        ch = toupper(ch);
                    else
                        ch = tolower(ch);
                    if (name.size() == 10 && name[9] == '|')
                        name[9] = ch;
                    else if (name.size() < 10) {
                        name[name.size() - 1] = ch;
                        name += "|";
                    }
                    _name.setString(name);
                    setText(_name, width / 2, height / 2 - 45);
                }
        }
        window.clear(Color::Blue);
        window.draw(welcome);
        window.draw(welcome2);
        window.draw(_name);
        window.display();
    }
    if (cont) {
        map<string, Texture> textures;
        Texture texture;
        texture.loadFromFile("files/images/debug.png");
        textures["debug"] = texture;
        texture.loadFromFile("files/images/digits.png");
        textures["digits"] = texture;
        texture.loadFromFile("files/images/face_happy.png");
        textures["happy"] = texture;
        texture.loadFromFile("files/images/face_lose.png");
        textures["lose"] = texture;
        texture.loadFromFile("files/images/face_win.png");
        textures["win"] = texture;
        texture.loadFromFile("files/images/flag.png");
        textures["flag"] = texture;
        texture.loadFromFile("files/images/leaderboard.png");
        textures["leaderboard"] = texture;
        texture.loadFromFile("files/images/mine.png");
        textures["mine"] = texture;
        for (int i = 1; i < 9; i++) {
            texture.loadFromFile("files/images/number_" + to_string(i) + ".png");
            textures["num" + to_string(i)] = texture;
        }
        texture.loadFromFile("files/images/pause.png");
        textures["pause"] = texture;
        texture.loadFromFile("files/images/play.png");
        textures["play"] = texture;
        texture.loadFromFile("files/images/tile_hidden.png");
        textures["hidden"] = texture;
        texture.loadFromFile("files/images/tile_revealed.png");
        textures["revealed"] = texture;
        bool gameOver = false;
        Text title("LEADERBOARD", font, 20);
        title.setFillColor(Color::White);
        title.setStyle(Text::Bold);
        title.setStyle(Text::Underlined);
        string leaders;
        Text _leaders(leaders, font, 18);
        _leaders.setFillColor(Color::White);
        _leaders.setStyle(Text::Bold);
        updateLeaders(6000, leaders, name, gameOver);
        RenderWindow gameWindow(VideoMode(col * 32, row * 32 + 100), "Minesweeper", Style::Close);
        Sprite happy;
        happy.setPosition(col / 2.0 * 32 - 32, 32 * (row + 0.5));
        happy.setTexture(textures["happy"]);
        Sprite debug;
        debug.setPosition(col * 32 - 304, 32 * (row + 0.5));
        debug.setTexture(textures["debug"]);
        bool debugged = false;
        Sprite play_pause;
        play_pause.setPosition(col * 32 - 240, 32 * (row + 0.5));
        play_pause.setTexture(textures["pause"]);
        bool paused = false;
        Sprite leaderboard;
        leaderboard.setPosition(col * 32 - 176, 32 * (row + 0.5));
        leaderboard.setTexture(textures["leaderboard"]);
        bool boardOpen = false;
        vector<vector<Tile>> board(row);
        for (int i = 0; i < row; i++) {
            vector<Tile> rowVect(col);
            for (int j = 0; j < col; j++) {
                Tile tile(i, j);
                tile.sprite.setPosition(j * 32, i * 32);
                rowVect[j] = tile;
            }
            board[i] = rowVect;
        }
        setBoard(board, mines, textures);
        auto start = chrono::high_resolution_clock::now();
        auto zero = start;
        float prev_time = 0.0;
        float elapsed_time = 0.0;
        while (gameWindow.isOpen()) {
            Event event;
            while (gameWindow.pollEvent(event)) {
                if (event.type == Event::Closed)
                    gameWindow.close();
                if (event.type == Event::MouseButtonPressed) {
                    Vector2f mouse = gameWindow.mapPixelToCoords(Mouse::getPosition(gameWindow));
                    if (event.mouseButton.button == Mouse::Left) {
                        FloatRect leaderButton = leaderboard.getGlobalBounds();
                        if (leaderButton.contains(mouse)) {
                            boardOpen = true;
                            paused = true;
                        }
                        if (!gameOver) {
                            FloatRect pauseButton = play_pause.getGlobalBounds();
                            if (pauseButton.contains(mouse)) {
                                paused = !paused;
                                if (!paused)
                                    play_pause.setTexture(textures["pause"]);
                                else
                                    play_pause.setTexture(textures["play"]);
                            }
                        }
                        if ((!paused || gameOver) && !boardOpen) {
                            FloatRect happyButton = happy.getGlobalBounds();
                            if (happyButton.contains(mouse)) {
                                happy.setTexture(textures["happy"]);
                                flags = mines;
                                numRevealed = 0;
                                start = chrono::high_resolution_clock::now();
                                prev_time = 0;
                                setBoard(board, mines, textures);
                                paused = false;
                                gameOver = false;
                                debugged = false;
                            }
                        }
                        if (!paused && !gameOver) {
                            FloatRect debugButton = debug.getGlobalBounds();
                            if (debugButton.contains(mouse))
                                debugged = !debugged;
                            for (int i = 0; i < row; i++) {
                                for (int j = 0; j < col; j++) {
                                    FloatRect tileButton = (board[i])[j].sprite.getGlobalBounds();
                                    if (tileButton.contains(mouse) && !(board[i])[j].isFlagged &&
                                        !(board[i])[j].isRevealed) {
                                        if ((board[i])[j].hasMine) {
                                            happy.setTexture(textures["lose"]);
                                            boardOpen = true;
                                            gameOver = true;
                                        } else if ((board[i])[j].numMinesAround > 0) {
                                            numRevealed++;
                                            (board[i])[j].isRevealed = true;
                                        } else if ((board[i])[j].numMinesAround == 0) {
                                            checkTiles((board[i])[j], numRevealed);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (event.mouseButton.button == Mouse::Right)
                        if (!paused && !gameOver)
                            for (int i = 0; i < row; i++)
                                for (int j = 0; j < col; j++) {
                                    FloatRect tileButton = (board[i])[j].sprite.getGlobalBounds();
                                    if (tileButton.contains(mouse))
                                        if (!(board[i])[j].isRevealed) {
                                            if ((board[i])[j].isFlagged)
                                                flags++;
                                            else
                                                flags--;
                                            (board[i])[j].isFlagged = !(board[i])[j].isFlagged;
                                        }
                                }
                }
            }
            gameWindow.clear(Color::White);
            if (numRevealed == row * col - mines) {
                happy.setTexture(textures["win"]);
                boardOpen = true;
                gameOver = true;
                updateLeaders((int) elapsed_time, leaders, name, gameOver);
            }
            if (gameOver && numRevealed == row * col - mines) {
                for (int i = 0; i < row; i++)
                    for (int j = 0; j < col; j++)
                        if ((board[i])[j].hasMine)
                            (board[i])[j].isFlagged = true;
            } else if (gameOver && numRevealed < row * col - mines)
                for (int i = 0; i < row; i++)
                    for (int j = 0; j < col; j++)
                        if ((board[i])[j].hasMine)
                            (board[i])[j].isRevealed = true;
            for (int i = 0; i < row; i++) {
                for (int j = 0; j < col; j++) {
                    gameWindow.draw((board[i])[j].sprite);
                    if ((board[i])[j].isRevealed)
                        (board[i])[j].sprite.setTexture(textures["revealed"]);
                    if ((board[i])[j].hasMine && (board[i])[j].isRevealed) {
                        Sprite mine;
                        mine.setPosition((float) j * 32, (float) i * 32);
                        mine.setTexture(textures["mine"]);
                        gameWindow.draw(mine);
                    } else if ((board[i])[j].isFlagged) {
                        Sprite flag;
                        flag.setPosition((float) j * 32, (float) i * 32);
                        flag.setTexture(textures["flag"]);
                        gameWindow.draw(flag);
                    } else if ((board[i])[j].numMinesAround > 0 && (board[i])[j].isRevealed) {
                        Sprite num;
                        num.setPosition(j * 32, i * 32);
                        num.setTexture(textures["num" + to_string((board[i])[j].numMinesAround)]);
                        gameWindow.draw(num);
                    }
                }
            }
            if (paused) {
                for (int i = 0; i < row; i++)
                    for (int j = 0; j < col; j++) {
                        Sprite tile;
                        tile.setPosition((float) j * 32, (float) i * 32);
                        tile.setTexture(textures["revealed"]);
                        gameWindow.draw(tile);
                    }
            } else if (debugged) {
                for (int i = 0; i < row; i++)
                    for (int j = 0; j < col; j++)
                        if ((board[i])[j].hasMine && !(board[i])[j].isRevealed) {
                            Sprite tile;
                            tile.setPosition((float) j * 32, (float) i * 32);
                            tile.setTexture(textures["mine"]);
                            gameWindow.draw(tile);
                        }
            }
            gameWindow.draw(happy);
            gameWindow.draw(debug);
            gameWindow.draw(play_pause);
            gameWindow.draw(leaderboard);
            Sprite digit;
            if (flags < 0) {
                digit.setPosition(12, 32 * (row + 0.5) + 16);
                digit.setTexture(textures["digits"]);
                digit.setTextureRect(IntRect(210, 0, 21, 32));
                gameWindow.draw(digit);
            }
            digit.setPosition(33, 32 * (row + 0.5) + 16);
            digit.setTexture(textures["digits"]);
            digit.setTextureRect(IntRect(abs(flags / 100) * 21, 0, 21, 32));
            gameWindow.draw(digit);
            digit.setPosition(54, 32 * (row + 0.5) + 16);
            digit.setTexture(textures["digits"]);
            digit.setTextureRect(IntRect(abs((flags % 100) / 10) * 21, 0, 21, 32));
            gameWindow.draw(digit);
            digit.setPosition(75, 32 * (row + 0.5) + 16);
            digit.setTexture(textures["digits"]);
            digit.setTextureRect(IntRect(abs(flags % 10) * 21, 0, 21, 32));
            gameWindow.draw(digit);
            auto end = chrono::high_resolution_clock::now();
            if (!paused)
                elapsed_time = chrono::duration_cast<chrono::seconds>(end - start).count() + prev_time;
            if (paused || gameOver) {
                start = end;
                prev_time = elapsed_time;
            }
            digit.setPosition((col * 32) - 97, 32 * (row + 0.5) + 16);
            digit.setTexture(textures["digits"]);
            digit.setTextureRect(IntRect((int) elapsed_time / 60 / 10 * 21, 0, 21, 32));
            gameWindow.draw(digit);
            digit.setPosition((col * 32) - 76, 32 * (row + 0.5) + 16);
            digit.setTexture(textures["digits"]);
            digit.setTextureRect(IntRect((int) elapsed_time / 60 % 10 * 21, 0, 21, 32));
            gameWindow.draw(digit);
            digit.setPosition((col * 32) - 54, 32 * (row + 0.5) + 16);
            digit.setTexture(textures["digits"]);
            digit.setTextureRect(IntRect((int) elapsed_time % 60 / 10 * 21, 0, 21, 32));
            gameWindow.draw(digit);
            digit.setPosition((col * 32) - 33, 32 * (row + 0.5) + 16);
            digit.setTexture(textures["digits"]);
            digit.setTextureRect(IntRect((int) elapsed_time % 60 % 10 * 21, 0, 21, 32));
            gameWindow.draw(digit);
            gameWindow.display();
            if (boardOpen) {
                _leaders.setString(leaders);
                RenderWindow leaderWindow(VideoMode(col * 16, row * 16 + 50), "Leaderboard", Style::Close);
                while (leaderWindow.isOpen()) {
                    Event event;
                    while (leaderWindow.pollEvent(event))
                        if (event.type == Event::Closed) {
                            leaderWindow.close();
                            numRevealed++;
                            boardOpen = false;
                            paused = false;
                            ifstream leaderFile("files/leaderboard.txt");
                            string lead;
                            char ch;
                            while (leaderFile.get(ch))
                                if (ch != '*')
                                    lead += ch;
                            ofstream output("files/leaderboard.txt");
                            output << lead;
                        }
                    end = chrono::high_resolution_clock::now();
                    start = end;
                    prev_time = elapsed_time;
                    leaderWindow.clear(Color::Blue);
                    setText(title, leaderWindow.getSize().x / 2, leaderWindow.getSize().y / 2 - 120);
                    setText(_leaders, leaderWindow.getSize().x / 2, leaderWindow.getSize().y / 2 + 20);
                    leaderWindow.draw(title);
                    leaderWindow.draw(_leaders);
                    leaderWindow.display();
                }
            }
        }
    }
    return 0;
}
