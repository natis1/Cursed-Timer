/*
 * Cursed Timer. Time yourself with a terminal.
 * Copyright (C) 2019 Eli Stone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "display.h"
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <cmath>
#include <iomanip>


void display::signalHandler(int sig) {
    if (sig == SIGKILL || sig == SIGSEGV) {
        endwin();
        return;
    } else if (sig == SIGTSTP) {
        std::cout << "Timer display paused. Resume with the \"fg\" command. Note: Your timer is still running." << std::endl;
        endwin();
        return;
    }
}

int display::getDir(std::string dir, std::vector<std::string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        std::cout << "Error(" << errno << ") opening " << dir << std::endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        if (strncmp(dirp->d_name, ".", 1) == 0)
            continue;
        files.push_back(std::string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}

void display::startDisplay()
{
    std::string saveFolder = std::getenv("HOME");
    saveFolder += "/.config/cursedtimer/";
    std::vector<std::string> saveFile;
    if (getDir(saveFolder, saveFile) != 0) {
        mkdir(saveFolder.c_str(), 0755);
    }
    std::cout << "Save file size is " << saveFile.size() << std::endl << "And the timer length is " << timeTotal;
    
    bool hasFile = (saveFile.size() < 1);
    if (hasFile) {
        hasFile = false;
        for (unsigned int i = 0; i < saveFile.size(); i++) {
            if (saveFile.at(i) == "timer.conf") {
                hasFile = true;
                break;
            }
        }

    }
    if (!hasFile) {
        config = {1000, 1000, 0, 0, 0, 0, 200, 800, 200, 400, 0, 0, 1000, 1000, 1000, 0, 0, 0};
        pseudojson::Value configFile = toJson(config);
        pseudojson::writeToFile(configFile, saveFolder + "timer.conf");
    }
    
    config = fromJson<display::configData>(pseudojson::fileToPseudoJson(saveFolder + "timer.conf"));
    
    colors[0] = config.text_foreground_r;
    colors[1] = config.text_foreground_g;
    colors[2] = config.text_foreground_b;
    colors[3] = config.text_background_r;
    colors[4] = config.text_background_g;
    colors[5] = config.text_background_b;
    colors[6] = config.bar_foreground_r;
    colors[7] = config.bar_foreground_g;
    colors[8] = config.bar_foreground_b;
    colors[9] = config.bar_background_r;
    colors[10] = config.bar_background_g;
    colors[11] = config.bar_background_b;
    colors[12] = config.done_foreground_r;
    colors[13] = config.done_foreground_g;
    colors[14] = config.done_foreground_b;
    colors[15] = config.done_background_r;
    colors[16] = config.done_background_g;
    colors[17] = config.done_background_b;
    initscr();
    cbreak();
    noecho();
    start_color();
    curs_set(0);
    signal(SIGKILL, display::signalHandler);
    signal(SIGTSTP, display::signalHandler);
    signal(SIGSEGV, display::signalHandler);
    
    if (LINES < 4 || COLS < 4) {
        return;
    }
    
    // The colors of the text
    init_color(65, colors[0], colors[1], colors[2]);
    init_color(66, colors[3], colors[4], colors[5]);
    
    // Colors of the partially filled bar
    init_color(67, colors[6], colors[7], colors[8]);
    init_color(68, colors[9], colors[10], colors[11]);
    
    // Colors of the full bar
    init_color(69, colors[12], colors[13], colors[14]);
    init_color(70, colors[15], colors[16], colors[17]);
    
    init_pair(2, 65, 66);
    init_pair(3, 67, 68);
    
    // Color pair 4 and 5 is inverted because ncurses lacks the full bar character. so instead we use spaces but anti-colored.
    init_pair(4, 70, 69);
    init_pair(5, 68, 67);
    
    nodelay(stdscr, true);
    attron(COLOR_PAIR(2));
    move(0, 0);
    clrtobot();
    move(0, 0);
    
    timerLoop();
    endTimer();
}

void display::rectangle(int y1, int x1, int y2, int x2)
{
    mvhline(y1, x1, 0, x2-x1);
    mvhline(y2, x1, 0, x2-x1);
    mvvline(y1, x1, 0, y2-y1);
    mvvline(y1, x2, 0, y2-y1);
    mvaddch(y1, x1, ACS_ULCORNER);
    mvaddch(y2, x1, ACS_LLCORNER);
    mvaddch(y1, x2, ACS_URCORNER);
    mvaddch(y2, x2, ACS_LRCORNER);
}

std::string display::formatTime(double time)
{
    std::string tOut;
    int days = time / 86400;
    time = std::fmod(time, 86400.0);
    int hours = time / 3600;
    time = std::fmod(time, 3600.0);
    int minutes = time / 60;
    time = std::fmod(time, 60.0);
    
    if (days > 0) {
        tOut += std::to_string(days) + ":";
    }
    if (hours > 9) {
        tOut += std::to_string(hours) + ":";
    } else if (hours > 0) {
        tOut += "0" + std::to_string(hours) + ":";
    }
    if (minutes > 9) {
        tOut += std::to_string(minutes) + ":";
    } else {
        tOut += "0" + std::to_string(minutes) + ":";
    }
    if (time < 10) {
        tOut += "0";
    }
    
    // Create an output string stream
    std::ostringstream streamObj3;
    streamObj3 << std::fixed;
    streamObj3 << std::setprecision(2);
    streamObj3 << time;
    tOut += streamObj3.str();
    
    return tOut;
}


void display::timerLoop()
{
    auto start = std::chrono::system_clock::now();
    std::string endTime = formatTime(timeTotal);
    while (true) {
        auto curTime = std::chrono::system_clock::now();
        std::chrono::duration<double> t = curTime - start;
        if (t.count() > timeTotal) {
            return;
        } else {
            double percentageBar = t.count() / timeTotal;
            int barSize = (int) (((COLS - 2) * percentageBar) + 0.5);
            
            attron(COLOR_PAIR(2));
            rectangle(0, 0, 4, COLS - 1);
            int leftMid = (COLS / 2) - (timerName.length() / 2);
            if (leftMid > 0) {
                move(1, leftMid);
                printw(timerName.c_str());
            }
            attron(COLOR_PAIR(5));
            std::string bar (barSize, ' ');
            std::string barEnd (COLS - 2 - barSize, ' ');
            move (2, 1);
            printw(bar.c_str());
            attron(COLOR_PAIR(3));
            printw(barEnd.c_str());
            move (3, 1);
            attron(COLOR_PAIR(2));
            printw(formatTime(t.count()).c_str());
            
            if (COLS - 1 - endTime.length() > 0) {
                move (3, COLS - 1 - endTime.length());
                printw(endTime.c_str());
            }
            
            refresh();
            // meme
            
        }
        usleep(100000);
        move(0, 0);
        clrtobot();
    }
}

void display::endTimer()
{
    std::string endTime = formatTime(timeTotal);
    while (true) {
        
        int barSize = (COLS - 2);
        
        attron(COLOR_PAIR(2));
        rectangle(0, 0, 4, COLS - 1);
        int leftMid = (COLS / 2) - (timerName.length() / 2);
        if (leftMid > 0) {
            move(1, leftMid);
            printw(timerName.c_str());
        }
        attron(COLOR_PAIR(4));
        std::string bar (barSize, ' ');
        move (2, 1);
        printw(bar.c_str());
        move (3, 1);
        attron(COLOR_PAIR(2));
        printw(endTime.c_str());
        
        if (COLS - 1 - endTime.length() > 0) {
            move (3, COLS - 1 - endTime.length());
            printw(endTime.c_str());
        }
        
        refresh();
        usleep(100000);
        move(0, 0);
        clrtobot();
        
        int c = getch();
        
        if (c == KEY_ENTER || c == 10) {
            curs_set(1);
            endwin();
            std::cout << "Your timer for " << timerName << " is complete!" << std::endl <<"Thank you for using cursed timer." << std::endl;
            return;
        }
    }
}
