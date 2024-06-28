//  _    _ _                _                 _ 
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// Class wrapper around a task that should emit a
// Qt signal at the end of the work
//
// This class gets the entrypoint passed in the beginning
// An starts executing when execute_work() is called
// this is perfect with a callback for threads
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuverink

#pragma once
#include <functional>
#include <qobject.h>
#include <qobjectdefs.h>


class Worker : public QObject {
    Q_OBJECT

public:
    /************************************************************
    *                       functions
    ************************************************************/

    // Constructor
    // @param the netry point for the wowrker
    Worker(std::function<void()> entry);

    // Destructor
    ~Worker();

    // Execute the work passed in the constructor
    void execute_work();

signals:
    /************************************************************
    *                       signals
    ************************************************************/

    // Emited at the end of execute_work signal
    void work_done();

private:
    /************************************************************
    *                       fields
    ************************************************************/

    std::function<void()> m_entry;  // Store the function entry point passed by constructor
};
