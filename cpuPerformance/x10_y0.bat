set var=x10_y0

rem openNotification.bat

start "" "G:\Jobs\codes\sysdev\scada\src\unitTest\rtdbTestSuite\build-performanceTest-vc2008_Qt4_8_5-Release\release\performanceTest.exe" " " "x10_y0"

start "" "python" cpuUsageMonitor.pyw rtdb "%var%"
start "" "python" cpuUsageMonitor.pyw performanceTest "%var%"
start "" "python" cpuUsageMonitor.pyw cpuUsage "%var%" 1

rem python cpuUsageMonitor.pyw rtdb var
rem python cpuUsageMonitor.pyw performanceTest var
rem python cpuUsageMonitor.pyw cpuUsage var