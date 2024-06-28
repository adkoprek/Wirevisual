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
// this is perfect as a callback for threads
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuverink

#include <functional>
#include <qobjectdefs.h>

#include "worker.h"


/************************************************************
*                       public
************************************************************/

// Constructor
Worker::Worker(std::function<void()> entry) {
    m_entry = entry;
}

// Destructor
Worker::~Worker() {}

// Execute the work passed in the constructor
void Worker::execute_work() {
    m_entry();
    emit work_done();           // Emit the work_done() signal to be captured in an other class
}
