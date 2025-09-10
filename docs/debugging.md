# Debugging
To debug packets being sent to and from the client you can use the following xml file, along with a launch arguement to enable it.


```XML
<?xml version="1.0" encoding="UTF-8"?>
<Configuration status="WARN" packages="com.mojang.util">
    <Appenders>
        <Console name="SysOut" target="SYSTEM_OUT">
            <PatternLayout pattern="[%d{HH:mm:ss}] [%t/%level]: %msg%n" />
        </Console>
        <Queue name="ServerGuiConsole">
            <PatternLayout pattern="[%d{HH:mm:ss} %level]: %msg%n" />
        </Queue>
        <RollingRandomAccessFile name="File" fileName="logs/latest.log" filePattern="logs/%d{yyyy-MM-dd}-%i.log.gz">
            <PatternLayout pattern="[%d{HH:mm:ss}] [%t/%level]: %msg%n" />
            <Policies>
                <TimeBasedTriggeringPolicy />
                <OnStartupTriggeringPolicy />
            </Policies>
            <DefaultRolloverStrategy max="1000"/>
        </RollingRandomAccessFile>
    </Appenders>
    <Loggers>
        <Root level="debug">
            <filters>
                <MarkerFilter marker="NETWORK_PACKETS" onMatch="ACCEPT" onMismatch="DENY" />
            </filters>
            <AppenderRef ref="SysOut"/>
            <AppenderRef ref="File"/>
            <AppenderRef ref="ServerGuiConsole"/>
        </Root>
    </Loggers>
</Configuration>
```

In your launcher of choice, use the following arguement to enable this XML debugging config  (Example below is on linux)

```-Dlog4j.configurationFile=/home/yourusername/MC++/log4j2.xml```