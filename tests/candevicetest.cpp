
#define CATCH_CONFIG_RUNNER
#include <fakeit.hpp>
#include <QtSerialBus/QCanBusFrame>

#include "candevice/candeviceinterface.hpp"
#include "candevice/canfactory.hpp"
#include "candevice/candeviceqt.hpp"
#include "candevice/canfactoryqt.hpp"
#include "candevice/candevice_p.h"
#include "log.hpp"

#define private public
#include <candevice/candevice.h>

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

TEST_CASE("Start failed","[candevice]")
{
    using namespace fakeit;
    Mock<CanFactoryInterface> factoryMock;
    Mock<CanDeviceQt> deviceQtMock;

    When(Method(factoryMock, create)).Return( nullptr );
    CanDevice canDeviceNullPtr{ factoryMock.get() };
    canDeviceNullPtr.init("","");
    CHECK(canDeviceNullPtr.start() == false);

    When(Method(deviceQtMock,connectDevice)).Return(false);
    Fake(Dtor(deviceQtMock),
         Method(deviceQtMock, framesWritten),
         Method(deviceQtMock, framesReceived),
         Method(deviceQtMock, errorOccurred));

    factoryMock.Reset();
    When(Method(factoryMock,create)).Return( &(deviceQtMock.get()) );

    CanDevice canDevice{ factoryMock.get() };
    canDevice.init("","");
    CHECK(canDevice.start() == false);
}

TEST_CASE("Start succeedded","[candevice]")
{
    using namespace fakeit;
    Mock<CanFactoryInterface> factoryMock;
    Mock<CanDeviceQt> deviceQtMock;

    When(Method(factoryMock, create)).Return( &(deviceQtMock.get()) );

    Fake(Dtor(deviceQtMock),
         Method(deviceQtMock, framesWritten),
         Method(deviceQtMock, framesReceived),
         Method(deviceQtMock, errorOccurred));

    When(Method(deviceQtMock, connectDevice)).Return(true);

    CanDevice canDevice{ factoryMock.get() };
    canDevice.init("","");
    CHECK(canDevice.start() == true);
}

TEST_CASE("framesReceived verification","[candevice]")
{
    Mock<CanFactoryInterface> factoryMock;
    Mock<CanDeviceQt> deviceQtMock;
    QCanBusFrame frame;

    When(Method(factoryMock, create)).Return( &(deviceQtMock.get()) );
    Fake(Dtor(deviceQtMock),
         Method(deviceQtMock, framesWritten),
         Method(deviceQtMock, framesReceived),
         Method(deviceQtMock, errorOccurred));
    When(Method(deviceQtMock, framesAvailable)).Return(10,9,8,7,6,5,4,3,2,1,0);
    When(Method(deviceQtMock, readFrame)).Return(10_Times(frame));

    CanDevice canDevice{ factoryMock.get() };
    canDevice.init("","");
    canDevice.framesReceived();
    Verify(Method(deviceQtMock, framesAvailable)).Exactly(11);
    Verify(Method(deviceQtMock, readFrame)).Exactly(10);
}

int main(int argc, char* argv[])
{
    bool haveDebug = std::getenv("CDS_DEBUG") != nullptr;
    kDefaultLogger = spdlog::stdout_color_mt("cds");
    if (haveDebug) {
        kDefaultLogger->set_level(spdlog::level::debug);
    }
    cds_debug("Startling unit tests");
    return Catch::Session().run(argc, argv);
}
