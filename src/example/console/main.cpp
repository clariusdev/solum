#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <atomic>
#include <thread>

#ifdef _MSC_VER
#include <boost/program_options.hpp>
#else
#include <unistd.h>
#endif

#include <oem/oem.h>

#define PRINT           std::cout << std::endl
#define PRINTSL         std::cout << "\r"
#define ERROR           std::cerr << std::endl
#define ERRCODE         (-1)
#define SUCCESS         (0)

// generic buffer for holding various retrievals
static std::string ip_;
static unsigned int port_ = 0;
static char buffer_[2048];
static int counter_ = 0;

/// callback for error messages
/// @param[in] err the error message sent from the oem module
void errorFn(const char* err)
{
    ERROR << "error: " << err;
}

/// callback for connection status
/// @param[in] ret connection status code
/// @param[in] port udp port used for streaming
/// @param[in] status the connection status message sent from the oem module
void connectFn(int ret, int port, const char* status)
{
    if (ret == CONNECT_ERROR)
        ERROR << "connection: " << ret << ", error: " << status;
    else
        PRINT << "connection: " << ret << ", status: " << status;

    if (ret == CONNECT_SUCCESS)
        PRINT << "streaming port: " << port;
}

/// callback for certification status
/// @param[in] daysValid # of days valid for certificate
void certFn(int daysValid)
{
    if (daysValid == CERT_INVALID)
        ERROR << "certificate invalid or not found";
    else if (!daysValid)
        ERROR << "certificate expired";
    else
        PRINT << "certificate valid for (" << daysValid << ") more days";
}

/// callback for probe powering down
/// @param[in] ret power down status code
/// @param[in] tm time when the probe is powering down
void powerDownFn(int ret, int tm)
{
    PRINT << "probe powering down: " << ret << ", in " << tm << "s";
}

/// callback for software updates
/// @param[in] ret software update status code
void swUpdateFn(int ret)
{
    if (ret == SWUPDATE_SUCCESS)
        PRINT << "software update was successful";
    else if (ret == SWUPDATE_CURRENT)
        PRINT << "software is already up to date";
    else
        ERROR << "error updating software: " << ret;
}

/// callback for imaging state change
/// @param[in] ready flag that probe is ready to image
/// @param[in] imaging 1 = running, 0 = stopped
void imagingFn(int ready, int imaging)
{
    if (ready == IMAGING_READY)
        PRINT << "ready to image: " << ((imaging) ? "imaging running" : "imaging stopped");
    else if (ready == IMAGING_CERTEXPIRED)
        ERROR << "certificate needs updating prior to imaging";
    else
        ERROR << "not ready to image";
}

/// callback for button press
/// @param[in] btn the button that was pressed, 0 = up, 1 = down
/// @param[in] clicks # of clicks used
void buttonFn(int btn, int clicks)
{
    PRINT << ((btn == BUTTON_DOWN) ? "down" : "up") << " button pressed, clicks: " << clicks;
}

/// callback for software update progress
/// @param[in] progress the update progress
void progressFn(int progress)
{
    PRINTSL << "updating: " << progress << "%" << std::flush;
}

/// prints imu data
/// @param[in] npos the # of positional data points embedded with the frame
/// @param[in] pos the buffer of positional data
void printImuData(int npos, const ClariusPosInfo* pos)
{
    for (auto i = 0; i < npos; i++)
    {
        PRINT << "imu: " << i << ", time: " << pos[i].tm;
        PRINT << "accel: " << pos[i].ax << "," << pos[i].ay << "," << pos[i].az;
        PRINT << "gyro: " << pos[i].gx << "," << pos[i].gy << "," << pos[i].gz;
        PRINT << "magnet: " << pos[i].mx << "," << pos[i].my << "," << pos[i].mz;
    }
}

/// parses and prints comma separated values
/// @param[in] buf the string to parse
/// @param[in] sz size of the buffer
/// @return success of the call
bool printCsv(const char* buf, int sz)
{
    if (sz > static_cast<int>(sizeof(buffer_)))
        return false;

    memcpy(buffer_, buf, sz);
    char* p = strtok(buffer_, ",");
    while (p != nullptr)
    {
        PRINT << p;
        p = strtok(nullptr, ",");
    }

    return true;
}

/// callback for a new pre-scan converted data sent from the scanner
/// @param[in] newImage a pointer to the raw image bits of
/// @param[in] nfo the image properties
/// @param[in] npos the # of positional data points embedded with the frame
/// @param[in] pos the buffer of positional data
void newRawImageFn(const void* newImage, const ClariusRawImageInfo* nfo, int npos, const ClariusPosInfo* pos)
{
#ifdef PRINTRAW
    if (nfo->rf)
        PRINT << "new rf data (" << newImage << "): " << nfo->lines << " x " << nfo->samples << " @ " << nfo->bitsPerSample
          << "bits. @ " << nfo->axialSize << " microns per sample. imu points: " << npos;
    else
        PRINT << "new pre-scan data (" << newImage << "): " << nfo->lines << " x " << nfo->samples << " @ " << nfo->bitsPerSample
          << "bits. @ " << nfo->axialSize << " microns per sample. imu points: " << npos << " jpeg size: " << (int)nfo->jpeg;

    if (npos)
        printImuData(npos, pos);
#else
    (void)newImage;
    (void)nfo;
    (void)npos;
    (void)pos;
#endif
}

/// callback for a new image sent from the scanner
/// @param[in] newImage a pointer to the raw image bits of
/// @param[in] nfo the image properties
/// @param[in] npos the # of positional data points embedded with the frame
/// @param[in] pos the buffer of positional data
void newProcessedImageFn(const void* newImage, const ClariusProcessedImageInfo* nfo, int npos, const ClariusPosInfo* pos)
{
    (void)newImage;
    PRINTSL << "new image (" << counter_++ << "): " << nfo->width << " x " << nfo->height << " @ " << nfo->bitsPerPixel << " bpp. @ "
            << nfo->imageSize << "bytes. @ " << nfo->micronsPerPixel << " microns per pixel. imu points: " << npos << std::flush;

    if (npos)
        printImuData(npos, pos);
}

/// processes the user input
/// @param[out] quit flag to shut down program
void processEventLoop(std::atomic_bool& quit)
{
    std::string cmd, buf1, buf2;
    ClariusStatusInfo stats;
    double v;
    int m;

    auto param = [](const std::string& p) -> int
    {
        if (p == "D" || p == "d")
            return PARAM_DEPTH;
        else if (p == "G" || p == "g")
            return PARAM_GAIN;
        else if (p == "I" || p == "i")
            return PARAM_IMU;

        return -1;
    };

    auto mode = [](const std::string& m) -> int
    {
        if (m == "B" || m == "b")
            return MODE_B;
        else if (m == "R" || m == "r")
            return MODE_RF;
        else if (m == "C" || m == "c")
            return MODE_CFI;
        else if (m == "P" || m == "p")
            return MODE_PDI;

        return -1;
    };

    PRINT << "enter command: ";
    while (std::getline(std::cin, cmd))
    {
        if (cmd == "Q" || cmd == "q")
        {
            quit = true;
            break;
        }
        else if (cmd == "C" || cmd == "c")
        {
            // ensure valid ip
            if (!ip_.size())
            {
                PRINT << "enter ip address: ";
                std::getline(std::cin, ip_);
            }
            // ensure valid port
            if (!port_)
            {
                PRINT << "enter port: ";
                std::getline(std::cin, buf1);
                try { port_ = std::stoi(buf1); }
                catch (std::exception&) { ERROR << "invalid port specified"; }
            }
            if (cusOemConnect(ip_.c_str(), port_) < 0)
                ERROR << "error calling connect";
            else
                PRINT << "trying to connect";
        }
        else if (cmd == "D" || cmd == "d")
        {
            // disconnect and reset ip/port
            cusOemDisconnect();
            ip_ = "";
            port_ = 0;
        }
        else if (cmd == "U" || cmd == "u")
        {
            if (cusOemSoftwareUpdate(swUpdateFn, progressFn) < 0)
                ERROR << "error requesting software update";
        }
        else if (cmd == "G" || cmd == "g")
        {
            if (cusOemStatusInfo(&stats) == 0)
                PRINT << "battery: " << stats.battery << "%, temperature: " << stats.temperature << "%";
        }
        else if (cmd == "P" || cmd == "p")
        {
            if (cusOemProbes([](const char* list, int sz)
            {
                PRINT << "probes:";
                printCsv(list, sz);
            }) < 0)
                ERROR << "error requesting probes";
        }
        else if (cmd == "A" || cmd == "a")
        {
            PRINT << "enter probe model: ";
            std::getline(std::cin, buf1);
            PRINT << "applications for " << buf1 << ":";
            if (cusOemApplications(buf1.c_str(), [](const char* list, int sz)
            {
                printCsv(list, sz);
            }) < 0)
                ERROR << "error requesting applications";
        }
        else if (cmd == "L" || cmd == "l")
        {
            PRINT << "enter probe model: ";
            std::getline(std::cin, buf1);
            PRINT << "enter application: ";
            std::getline(std::cin, buf2);
            if (cusOemLoadApplication(buf1.c_str(), buf2.c_str()) == 0)
                PRINT << "trying to load application: " << buf2;
            else
                ERROR << "error calling load application";
        }
        else if (cmd == "R" || cmd == "r")
        {
            counter_ = 0;
            if (cusOemRun(1) < 0)
                ERROR << "run request failed";
        }
        else if (cmd == "S" || cmd == "s")
        {
            if (cusOemRun(0) < 0)
                ERROR << "stop request failed";
        }
        else if (cmd == "F" || cmd == "f")
        {
            PRINT << "select parameter [d=depth, g=gain, i=imu]: ";
            std::getline(std::cin, buf1);
            v = cusOemGetParam(param(buf1));
            if (v == -1)
                ERROR << "parameter request failed";
            else
                PRINT << "value: " << v;
        }
        else if (cmd == "V" || cmd == "v")
        {
            PRINT << "select parameter [d=depth, g=gain, i=imu]: ";
            std::getline(std::cin, buf1);
            PRINT << "set value: ";
            std::getline(std::cin, buf2);
            v = std::atoi(buf2.c_str());
            if (cusOemSetParam(param(buf1), v) < 0)
                ERROR << "error setting parameter";
        }
        else if (cmd == "N" || cmd == "n")
        {
            if ((m = cusOemGetMode()) < 0)
                ERROR << "mode retrieval failed";
            else if (m == MODE_B)
                PRINT << "b mode";
            else if (m == MODE_RF)
                PRINT << "rf mode";
            else if (m == MODE_CFI)
                PRINT << "cfi mode";
            else if (m == MODE_PDI)
                PRINT << "pdi mode";
        }
        else if (cmd == "M" || cmd == "m")
        {
            PRINT << "select mode [b=b mode, r=rf, c=cfi, p=pdi]: ";
            std::getline(std::cin, buf1);
            if (cusOemSetMode(mode(buf1)) < 0)
                ERROR << "mode request failed";
        }
        else
        {
            PRINT << "valid commands: [q: quit, h: help]";
            PRINT << "    connection: [c: connect, d: disconnect, u: software update, g: get status]";
            PRINT << "      workflow: [p: list probes, a: list applications, l: load workflow]";
            PRINT << "       imaging: [r: run imaging, s: stop imaging ]";
            PRINT << "    parameters: [f: fetch parameter, v: set parameter value ]";
            PRINT << "  imaging mode: [n: fetch mode, m: set mode ]";
        }
        PRINT << "enter command: ";
    }
}

int init(int& argc, char** argv)
{
    const int width  = 640;
    const int height = 480;
    // ensure console buffers are flushed automatically
    setvbuf(stdout, nullptr, _IONBF, 0) != 0 || setvbuf(stderr, nullptr, _IONBF, 0);

    // msvc doesn't have 'getopt' so use boost program_options instead
#ifdef _MSC_VER
    namespace po = boost::program_options;
    std::string keydir;

    try
    {
        po::options_description desc("Usage: 192.168.1.1", 12345);
        desc.add_options()
            ("help", "produce help message")
            ("address", po::value<std::string>(&ip_), "set the IP address of the host scanner")
            ("port", po::value<unsigned int>(&port_), "set the port of the host scanner")
            ("keydir", po::value<std::string>(&keydir)->default_value("/tmp/"), "set the path containing the security keys")
        ;

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);

        if (vm.count("help"))
        {
            PRINT << desc;
            return ERRCODE;
        }

        po::notify(vm);
    }
    catch(std::exception& e)
    {
        ERROR << "error: " << e.what();
        return ERRCODE;
    }
    catch(...)
    {
        ERROR << "unknown error";
        return ERRCODE;
    }
#else // every other platform has 'getopt' which we're using so as to not pull in the Boost dependency
    int o;
    std::string keydir = "/tmp/";

    // check command line options
    while ((o = getopt(argc, argv, "k:a:p:")) != -1)
    {
        switch (o)
        {
        // security key directory
        case 'k': keydir = optarg; break;
        // ip address
        case 'a': ip_ = optarg; break;
        // port
        case 'p':
            try { port_ = std::stoi(optarg); }
            catch (std::exception&) { PRINT << port_; }
            break;
        // invalid argument
        case '?': PRINT << "invalid argument, valid options: -a [addr], -p [port], -k [keydir]"; break;
        default: break;
        }
    }    
#endif

    // ensure an ip address is specified with the port
    if (port_ && !ip_.size())
    {
        ERROR << "no ip address provided with port provided. run with '-a [addr] -p [port]" << std::endl;
        return ERRCODE;
    }

    PRINT << "starting clarius oem program...";

    // initialize with callbacks
    if (cusOemInit(argc, argv, keydir.c_str(), connectFn, certFn, powerDownFn, newProcessedImageFn,
                   newRawImageFn, nullptr, imagingFn, buttonFn, errorFn, width, height) < 0)
    {
        ERROR << "could not initialize oem module" << std::endl;
        return ERRCODE;
    }

    // try and connect right away if parameters provided
    if (ip_.size() && port_)
    {
        if (cusOemConnect(ip_.c_str(), port_) < 0)
            ERROR << "error calling connect";
        else
            PRINT << "trying to connect";
    }

    return 0;
}

/// main entry point
/// @param[in] argc # of program arguments
/// @param[in] argv list of arguments
int main(int argc, char* argv[])
{
    int rcode = init(argc, argv);

    if (rcode != SUCCESS)
        return rcode;

    std::atomic_bool quitFlag(false);
    std::thread eventLoop(processEventLoop, std::ref(quitFlag));
    eventLoop.join();
    cusOemDestroy();
    return rcode;
}
