#include <QTest>
#include "core/playerengine.h"

class TestPlayerEngine : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testInitialState();
    void testSetVolume();
    void cleanupTestCase();
};

void TestPlayerEngine::initTestCase()
{
}

void TestPlayerEngine::testInitialState()
{
    PlayerEngine engine;
    QCOMPARE(engine.playbackState(), PlayerEngine::Stopped);
}

void TestPlayerEngine::testSetVolume()
{
    PlayerEngine engine;
    engine.setVolume(0.5f);
    // Volume should be clamped between 0 and 1
    engine.setVolume(1.5f);
    engine.setVolume(-0.1f);
}

void TestPlayerEngine::cleanupTestCase()
{
}

QTEST_MAIN(TestPlayerEngine)
#include "test_playerengine.moc"
