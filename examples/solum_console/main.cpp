#include <cstdio>
#include <cstring>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <atomic>
#include <thread>

#ifdef _MSC_VER
#include <boost/program_options.hpp>
#else
#include <unistd.h>
#endif

#include <solum/solum.h>

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
/// @param[in] code the error code
/// @param[in] err the error message sent from the solum module
void errorFn(CusErrorCode code, const char* err)
{
    ERROR << "error: (" << static_cast<int>(code) << ") " << err;
}

/// callback for connection status
/// @param[in] res connection result
/// @param[in] port udp port used for streaming
/// @param[in] status the connection status message sent from the solum module
void connectFn(CusConnection res, int port, const char* status)
{
    if (res == ConnectionError)
        ERROR << "connection: " << res << ", error: " << status;
    else
        PRINT << "connection: " << res << ", status: " << status;

    if (res == ProbeConnected)
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
/// @param[in] res power down reason
/// @param[in] tm time when the probe is powering down
void powerDownFn(CusPowerDown res, int tm)
{
    PRINT << "probe powering down: " << static_cast<int>(res) << ", in " << tm << "s";
}

/// callback for software updates
/// @param[in] res software update result
void swUpdateFn(CusSwUpdate res)
{
    if (res == SwUpdateSuccess)
        PRINT << "software update was successful";
    else if (res == SwUpdateCurrent)
        PRINT << "software is already up to date";
    else
        ERROR << "error updating software: " << static_cast<int>(res);
}

/// callback for imaging state change
/// @param[in] state imaging ready state
/// @param[in] imaging 1 = running, 0 = stopped
void imagingFn(CusImagingState state, int imaging)
{
    if (state == ImagingReady)
        PRINT << "ready to image: " << ((imaging) ? "imaging running" : "imaging stopped");
    else if (state == CertExpired)
        ERROR << "certificate needs updating prior to imaging";
    else
        ERROR << "not ready to image";
}

/// callback for button press
/// @param[in] btn the button that was pressed
/// @param[in] clicks # of clicks used
void buttonFn(CusButton btn, int clicks)
{
    PRINT << ((btn == ButtonDown) ? "down" : "up") << " button pressed, clicks: " << clicks;
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
void printImuData(int npos, const CusPosInfo* pos)
{
    for (auto i = 0; i < npos; i++)
    {
        PRINT << "imu: " << i << ", time: " << pos[i].tm;
        PRINT << "accel: " << pos[i].ax << "," << pos[i].ay << "," << pos[i].az;
        PRINT << "gyro: " << pos[i].gx << "," << pos[i].gy << "," << pos[i].gz;
        PRINT << "magnet: " << pos[i].mx << "," << pos[i].my << "," << pos[i].mz;
    }
}

/// @brief Receives the update of the imu streaming port
/// @param port the new imu data UDP streaming port
void newImuPort(int port)
{
    if (port != 0)
    {
        PRINT << "imu now streaming at port: " << port;
    }
    else
    {
        PRINT << "imu streaming off";
    }
}

/// @brief Receives the new imu data streamed from the scanner
/// @param pos the positional information data streamed
void newImuData(const CusPosInfo* pos)
{
    PRINT << "imu data streamed:";
    printImuData(1, pos);
}

/// parses and prints comma separated values
/// @param[in] buf the string to parse
/// @param[in] sz size of the buffer
/// @return success of the call
bool printCsv(const char* buf, int sz)
{
    if (sz > static_cast<int>(sizeof(buffer_)))
        return false;

    std::memcpy(buffer_, buf, sz);
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
void newRawImageFn(const void* newImage, const CusRawImageInfo* nfo, int npos, const CusPosInfo* pos)
{
#ifdef PRINTRAW
    if (nfo->rf)
        PRINT << "new rf data (" << newImage << "): " << nfo->lines << " x " << nfo->samples << " @ " << nfo->bitsPerSample
          << "bits. @ " << nfo->axialSize << " microns per sample. imu points: " << npos;
    else
        PRINT << "new pre-scan data (" << newImage << "): " << nfo->lines << " x " << nfo->samples << " @ " << nfo->bitsPerSample
          << "bits. @ " << nfo->axialSize << " microns per sample. imu points: " << npos << " jpeg size: " << static_cast<int>(nfo->jpeg);

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
void newProcessedImageFn(const void* newImage, const CusProcessedImageInfo* nfo, int npos, const CusPosInfo* pos)
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
    CusStatusInfo stats;
    CusProbeInfo probe;
    auto connectParams = solumDefaultConnectionParams();
    double v;
    int m;

    auto param = [](const std::string& p) -> CusParam
    {
        if (p == "D" || p == "d")
            return ImageDepth;
        else if (p == "G" || p == "g")
            return Gain;
        else if (p == "I" || p == "i")
            return ImuStreaming;

        return Gain;
    };

    auto mode = [](const std::string& m) -> CusMode
    {
        if (m == "B" || m == "b")
            return BMode;
        else if (m == "R" || m == "r")
            return RfMode;
        else if (m == "C" || m == "c")
            return ColorMode;
        else if (m == "P" || m == "p")
            return PowerMode;

        return BMode;
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
            connectParams.ipAddress = ip_.c_str();
            connectParams.port = port_;
            if (solumConnect(&connectParams) < 0)
                ERROR << "error calling connect";
            else
                PRINT << "trying to connect";
        }
        else if (cmd == "D" || cmd == "d")
        {
            // disconnect and reset ip/port
            solumDisconnect();
            ip_ = "";
            port_ = 0;
        }
        else if (cmd == "U" || cmd == "u")
        {
            PRINT << "enter firmware path: ";
            std::getline(std::cin, buf1);
            if (solumSoftwareUpdate(buf1.c_str(), swUpdateFn, progressFn, 0) < 0)
                ERROR << "error requesting software update";
        }
        else if (cmd == "G" || cmd == "g")
        {
            if (solumStatusInfo(&stats) == 0)
                PRINT << "battery: " << stats.battery << "%, temperature: " << stats.temperature << "%, fr: " << stats.frameRate << "Hz";
            else
                ERROR << "error requesting status";
        }
        else if (cmd == "I" || cmd == "i")
        {
            if (solumProbeInfo(&probe) == 0)
                PRINT << "version: " << probe.version << ", elements: " << probe.elements
                      << ", pitch: " << probe.pitch << ", radius: " << probe.radius;
            else
                ERROR << "error requesting probe info";
        }
        else if (cmd == "X" || cmd == "x")
        {
            PRINT << "enter certificate path: ";
            std::getline(std::cin, buf1);

            std::ifstream fs(buf1);
            if (!fs.is_open())
                ERROR << "error loading certificate file";
            else
            {
                std::stringstream ss;
                ss << fs.rdbuf();
                if (solumSetCert(ss.str().c_str()) < 0)
                    ERROR << "error sending certificate";
            }
        }
        else if (cmd == "P" || cmd == "p")
        {
            if (solumProbes([](const char* list, int sz)
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
            if (solumApplications(buf1.c_str(), [](const char* list, int sz)
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
            if (solumLoadApplication(buf1.c_str(), buf2.c_str()) == 0)
                PRINT << "trying to load application: " << buf2;
            else
                ERROR << "error calling load application";
        }
        else if (cmd == "R" || cmd == "r")
        {
            counter_ = 0;
            if (solumRun(1) < 0)
                ERROR << "run request failed";
        }
        else if (cmd == "S" || cmd == "s")
        {
            if (solumRun(0) < 0)
                ERROR << "stop request failed";
        }
        else if (cmd == "F" || cmd == "f")
        {
            PRINT << "select parameter [d=depth, g=gain, i=imu]: ";
            std::getline(std::cin, buf1);
            v = solumGetParam(param(buf1));
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
            if (solumSetParam(param(buf1), v) < 0)
                ERROR << "error setting parameter";
        }
        else if (cmd == "N" || cmd == "n")
        {
            m = solumGetMode();
            if (m == BMode)
                PRINT << "b mode";
            else if (m == RfMode)
                PRINT << "rf mode";
            else if (m == ColorMode)
                PRINT << "cfi mode";
            else if (m == PowerMode)
                PRINT << "pdi mode";
        }
        else if (cmd == "M" || cmd == "m")
        {
            PRINT << "select mode [b=b mode, r=rf, c=cfi, p=pdi]: ";
            std::getline(std::cin, buf1);
            if (solumSetMode(mode(buf1)) < 0)
                ERROR << "mode request failed";
        }
        else
        {
            PRINT << "valid commands: [q: quit, h: help]";
            PRINT << "    connection: [c: connect, d: disconnect]";
            PRINT << "        manage: [x: load cert, u: software update, g: get status, i: get probe info]";
            PRINT << "      workflow: [p: list probes, a: list applications, l: load workflow]";
            PRINT << "       imaging: [r: run imaging, s: stop imaging ]";
            PRINT << "    parameters: [f: fetch parameter, v: set parameter value ]";
            PRINT << "  imaging mode: [n: fetch mode, m: set mode ]";
            PRINT << " ";
            PRINT << " typical usage: - power up and fetch tcp information via ble (not demonstrated here)";
            PRINT << "                - connect over wifi/tcp";
            PRINT << "                - load certificate";
            PRINT << "                - check for software update";
            PRINT << "                - load workflow";
            PRINT << "                - image, get status, adjust parameters";
        }
        PRINT << "enter command: ";
    }
}

int init(int& argc, char** argv)
{
    auto connectParams = solumDefaultConnectionParams();
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
    catch (std::exception& e)
    {
        ERROR << "error: " << e.what();
        return ERRCODE;
    }
    catch (...)
    {
        ERROR << "unknown error";
        return ERRCODE;
    }
#else // every other platform has 'getopt' which we're using so as to not pull in the Boost dependency
    int o;
    std::string keydir = "/tmp/";

    // check command line options
    while ((o = getopt(argc, argv, "lk:a:p:")) != -1)
    {
        switch (o)
        {
        // security key directory
        case 'k': keydir = optarg; break;
        // ip address
        case 'a': ip_ = optarg; break;
        // port
        case 'l':
            PRINT << "enabling solum logging";
            break;
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

    PRINT << "starting solum program...";

    auto initParams = solumDefaultInitParams();
    initParams.args.argc = argc;
    initParams.args.argv = argv;
    initParams.storeDir = keydir.c_str();
    initParams.connectFn = connectFn;
    initParams.certFn = certFn;
    initParams.powerDownFn = powerDownFn;
    initParams.newProcessedImageFn = newProcessedImageFn;
    initParams.newRawImageFn = newRawImageFn;
    initParams.newImuPortFn = newImuPort;
    initParams.newImuDataFn = newImuData;
    initParams.imagingFn = imagingFn;
    initParams.buttonFn = buttonFn;
    initParams.errorFn = errorFn;
    initParams.width = width;
    initParams.height = height;
    // initialize with callbacks
    if (solumInit(&initParams) < 0)
    {
        ERROR << "could not initialize solum module" << std::endl;
        return ERRCODE;
    }

    // try and connect right away if parameters provided
    if (ip_.size() && port_)
    {
        connectParams.ipAddress = ip_.c_str();
        connectParams.port = port_;
        if (solumConnect(&connectParams) < 0)
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
    solumDestroy();
    return rcode;
}
