# libidevice

A library to communicate with services on iOS devices using native protocols.

Based on [libimobiledevice](https://github.com/libimobiledevice/libimobiledevice), additionally implemented **instrument_service**.

​           


## Build

1. Compile the [libimobiledevice](https://github.com/libimobiledevice/libimobiledevice) project first, and then set its pkgconfig output directory to the `IMB_PKG_CONFIG_PATH` environment variable.

e.g.:
```bash
$ export IMB_PKG_CONFIG_PATH="../libimobiledevice/built/darwin-arm64/lib/pkgconfig"
```

2. Compile with CMake.

```bash
$ mkdir build && cd build
$ cmake ..
// or cmake -G "Xcode" ..
```

​             

## Usage

### libidevice

```c++
idevice_t device = ...;

DTXTransport* transport = new DTXTransport(device);
DTXConnection* connection = new DTXConnection(transport);
connection->Connect();

std::shared_ptr<DTXChannel> channel = connection->MakeChannelWithIdentifier(
    "com.apple.instruments.server.services.deviceinfo");
std::shared_ptr<DTXMessage> message = DTXMessage::Create("runningProcesses");
channel->SendMessageAsync(message,  [&](auto msg) {
    printf("reply handler\n");
    msg->Dump();
});
```

​          

### idevice
#### running_processes

Print the list of running processes on your iPhone:
```bash
$ idevice instruments running_processes
```

output:
```json
[
    {
        "isApplication": false,
        "name": "FamilyControlsAgent",
        "pid": 4417,
        "realAppName": "/System/Library/Frameworks/FamilyControls.framework/FamilyControlsAgent",
        "startDate": {
            "time": "1661865623.956983"
        }
    },
    {
        "isApplication": false,
        "name": "routined",
        "pid": 40,
        "realAppName": "/usr/libexec/routined",
        "startDate": {
            "time": "1661787079.115298"
        }
    }
]
```

​      

#### request_device_gpu_info

Query the GPU infomation of your iPhone:

```bash
$ idevice instruments request_device_gpu_info
```

output:

```json
[
    {
        "accelerator-id": 4294967943,
        "agx-tracecode-version": "3.24.4",
        "device-name": "A13",
        "displays": [
            {
                "accelerator-id": 4294967943,
                "built-in": true,
                "device-name": "LCD",
                "display-id": 1,
                "framebuffer-index": 0,
                "pixel-height": 2688,
                "pixel-width": 1242,
                "resolution": [
                    1242,
                    2688
                ]
            }
        ],
        "family-name": "A13",
        "headless": false,
        "low-power": false,
        "min-collection-interval": 0,
        "mobile": true,
        "perf-state": {
            "accelerator-id": 4294967943,
            "available": true,
            "enabled": false,
            "level": 0,
            "mapping": 16975360,
            "sustained": false
        },
        "product-name": "",
        "recommended-max-working-set-size": 2620473344,
        "removable": false,
        "supported-counter-profiles": [
            0,
            3,
            4,
            1
        ],
        "vendor-name": "Apple"
    }
]
```

​      

#### hardware_information

Query the hardware infomation of your iPhone:

```bash
$ idevice instruments hardware_information
```

output:

```json
{
    "hwCPU64BitCapable": 1,
    "hwCPUsubtype": 2,
    "hwCPUtype": 16777228,
    "numberOfCpus": 6,
    "numberOfPhysicalCpus": 6
}
```

​              

#### network_information

Query the network infomation of your iPhone:

```bash
$ idevice instruments network_information
```

output:

```json
{
    "en0": "Wi-Fi",
    "en1": "Ethernet Adapter (en1)",
    "en2": "Ethernet Adapter (en2)",
    "lo0": "Loopback"
}
```

​            

#### mach_time_info

Query the Mach time infomation of your iPhone:

```bash
$ idevice instruments mach_time_info
```

output:

```json
[
    1627750298767,
    125,
    3,
    3382495349528,
    1661927999.285198,
    "Asia/Shanghai"
]
```

​             

#### execname_for_pid

Get the binary executable name of the pid:

```bash
$ idevice instruments execname_for_pid --pid 1834
```

output:

```json
"/private/var/containers/Bundle/Application/36430233-EAA7-40DC-A9B8-25AD189342E6/WeChat.app/WeChat"
```

​                 

#### graphics_opengl

Get the profile data of graphics opengl in real-time:

```bash
$ idevice instruments graphics_opengl
```

output:

```json
{
    "Alloc system memory": 556269568,
    "Allocated PB Size": 3145728,
    "CoreAnimationFramesPerSecond": 0,
    "Device Utilization %": 0,
    "IOGLBundleName": "Built-In",
    "In use system memory": 16449536,
    "Renderer Utilization %": 0,
    "SplitSceneCount": 0,
    "TiledSceneBytes": 0,
    "Tiler Utilization %": 0,
    "XRVideoCardRunTimeStamp": 9164067,
    "recoveryCount": 0
}
{
    "Alloc system memory": 556269568,
    "Allocated PB Size": 3145728,
    "CoreAnimationFramesPerSecond": 0,
    "Device Utilization %": 0,
    "IOGLBundleName": "Built-In",
    "In use system memory": 16449536,
    "Renderer Utilization %": 0,
    "SplitSceneCount": 0,
    "TiledSceneBytes": 0,
    "Tiler Utilization %": 0,
    "XRVideoCardRunTimeStamp": 10180084,
    "recoveryCount": 0
}
```

​                                                             

#### decode

This command is used to decode the binary records of communication messages with Xcode:
```bash
$ idevice decode --hex received_outfile.bin transmit_outfile.bin
```

output:
```bash
==== DTXMessage ====
message_type: 2
identifier: 5100
conversation_index: 0
channel_code: 0
expects_reply: 1
auxiliary:
DTXPrimitiveArray, size=2: 
	item #0: [type=kSignedInt32, size=4, value=18]
	item #1: [type=kBuffer, size=184, value="com.apple.instruments.server.services.assets"]
  00000000h:  62 70 6C 69 73 74 30 30 D4 01 02 03 04 05 06 07  bplist00........
  00000010h:  0A 58 24 76 65 72 73 69 6F 6E 59 24 61 72 63 68  .X$versionY$arch
  00000020h:  69 76 65 72 54 24 74 6F 70 58 24 6F 62 6A 65 63  iverT$topX$objec
  00000030h:  74 73 12 00 01 86 A0 5F 10 0F 4E 53 4B 65 79 65  ts....._..NSKeye
  00000040h:  64 41 72 63 68 69 76 65 72 D1 08 09 54 72 6F 6F  dArchiver...Troo
  00000050h:  74 80 01 A2 0B 0C 55 24 6E 75 6C 6C 5F 10 2C 63  t.....U$null_.,c
  00000060h:  6F 6D 2E 61 70 70 6C 65 2E 69 6E 73 74 72 75 6D  om.apple.instrum
  00000070h:  65 6E 74 73 2E 73 65 72 76 65 72 2E 73 65 72 76  ents.server.serv
  00000080h:  69 63 65 73 2E 61 73 73 65 74 73 08 11 1A 24 29  ices.assets...$)
  00000090h:  32 37 49 4C 51 53 56 5C 00 00 00 00 00 00 01 01  27ILQSV\........
  000000a0h:  00 00 00 00 00 00 00 0D 00 00 00 00 00 00 00 00  ................
  000000b0h:  00 00 00 00 00 00 00 8B                          ........
payload(size=175):
  00000000h:  62 70 6C 69 73 74 30 30 D4 01 02 03 04 05 06 07  bplist00........
  00000010h:  0A 58 24 76 65 72 73 69 6F 6E 59 24 61 72 63 68  .X$versionY$arch
  00000020h:  69 76 65 72 54 24 74 6F 70 58 24 6F 62 6A 65 63  iverT$topX$objec
  00000030h:  74 73 12 00 01 86 A0 5F 10 0F 4E 53 4B 65 79 65  ts....._..NSKeye
  00000040h:  64 41 72 63 68 69 76 65 72 D1 08 09 54 72 6F 6F  dArchiver...Troo
  00000050h:  74 80 01 A2 0B 0C 55 24 6E 75 6C 6C 5F 10 23 5F  t.....U$null_.#_
  00000060h:  72 65 71 75 65 73 74 43 68 61 6E 6E 65 6C 57 69  requestChannelWi
  00000070h:  74 68 43 6F 64 65 3A 69 64 65 6E 74 69 66 69 65  thCode:identifie
  00000080h:  72 3A 08 11 1A 24 29 32 37 49 4C 51 53 56 5C 00  r:...$)27ILQSV\.
  00000090h:  00 00 00 00 00 01 01 00 00 00 00 00 00 00 0D 00  ................
  000000a0h:  00 00 00 00 00 00 00 00 00 00 00 00 00 00 82     ...............
"_requestChannelWithCode:identifier:"
==== /DTXMessage ====

==== DTXMessage ====
message_type: 0
identifier: 5100
conversation_index: 1
channel_code: 0
expects_reply: 0
auxiliary:
none
payload(size=0):
==== /DTXMessage ====
```

