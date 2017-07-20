
#include <candevice/candevice.h>
#define CATCH_CONFIG_RUNNER
#include <fakeit.hpp>

#include "candevice/candeviceinterface.hpp"
#include "candevice/canfactory.hpp"
#include "candevice/candeviceqt.hpp"
#include "candevice/canfactoryqt.hpp"

#include "log.hpp"

std::shared_ptr<spdlog::logger> kDefaultLogger;

using namespace fakeit;

TEST_CASE("Initialization failed", "[candevice]")
{

    Mock<CanFactoryInterface> factoryMock;
    When(Method(factoryMock, create)).Return(nullptr);
    CanDevice canDevice{ factoryMock.get() };
    CHECK(canDevice.init("", "") == false);
}

TEST_CASE("Initialization succedded", "[candevice]")
{
    Mock<CanFactoryInterface> factoryMock;
    Mock<CanDeviceInterface> deviceMock;

    Fake(Dtor(deviceMock));
    When(Method(deviceMock, framesWritten)).Do([](const auto& cb) { cb(100); });
    Fake(Method(deviceMock, framesReceived));
    Fake(Method(deviceMock, errorOccurred));

    When(Method(factoryMock, create)).Return(&(deviceMock.get()));
    CanDevice canDevice{ factoryMock.get() };
    CHECK(canDevice.init("", "") == true);
}

TEST_CASE("Start failed - canDevice is not initialized","[candevice]")
{
    Mock<CanFactoryInterface> factoryMock;
    When(Method(factoryMock, create)).Return(nullptr);
    CanDevice canDevice{ factoryMock.get() };
    CHECK(canDevice.start() == false);
}

TEST_CASE("Start failed - could not connect to device","[candevice]")
{
    Mock<CanDeviceQt> device;
    Mock<CanFactoryQt> canFactory;
    When(Method(device, connectDevice)).Return(false);
    When(Method(canFactory, create)).Return(&device.get());
    CanDevice canDevice(canFactory.get());
    CHECK(canDevice.start() == false);
}

int main(int argc, char* argv[])
{
    bool haveDebug = std::getenv("CDS_DEBUG") != nullptr;
    kDefaultLogger = spdlog::stdout_color_mt("cds");
    if (haveDebug) {
        kDefaultLogger->set_level(spdlog::level::debug);
    }
    cds_debug("Staring unit tests");
    return Catch::Session().run(argc, argv);
}
