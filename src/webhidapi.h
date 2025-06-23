// emcmake cmake ..\git -DCMAKE_BUILD_TYPE=Release
// ninja
// emrun examples\web\test.html

#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>
#include <iostream>
#include <malloc.h>

using namespace emscripten;

struct hid_device_;
typedef struct hid_device_ hid_device; /**< opaque hidapi structure */

/** @brief HID underlying bus types.

	@ingroup API
*/
typedef enum {
	/** Unknown bus type */
	HID_API_BUS_UNKNOWN = 0x00,

	/** USB bus
	   Specifications:
	   https://usb.org/hid */
	HID_API_BUS_USB = 0x01,

	/** Bluetooth or Bluetooth LE bus
		Specifications:
		https://www.bluetooth.com/specifications/specs/human-interface-device-profile-1-1-1/
		https://www.bluetooth.com/specifications/specs/hid-service-1-0/
		https://www.bluetooth.com/specifications/specs/hid-over-gatt-profile-1-0/ */
	HID_API_BUS_BLUETOOTH = 0x02,

	/** I2C bus
		Specifications:
		https://docs.microsoft.com/previous-versions/windows/hardware/design/dn642101(v=vs.85) */
	HID_API_BUS_I2C = 0x03,

	/** SPI bus
		Specifications:
		https://www.microsoft.com/download/details.aspx?id=103325 */
	HID_API_BUS_SPI = 0x04,
} hid_bus_type;

/** hidapi info structure */
struct hid_device_info {
	/** Platform-specific device path */
	char* path;
	/** Device Vendor ID */
	unsigned short vendor_id;
	/** Device Product ID */
	unsigned short product_id;
	/** Serial Number */
	wchar_t* serial_number;
	/** Device Release Number in binary-coded decimal,
		also known as Device Version Number */
	unsigned short release_number;
	/** Manufacturer String */
	wchar_t* manufacturer_string;
	/** Product string */
	wchar_t* product_string;
	/** Usage Page for this Device/Interface
		(Windows/Mac/hidraw only) */
	unsigned short usage_page;
	/** Usage for this Device/Interface
		(Windows/Mac/hidraw only) */
	unsigned short usage;
	/** The USB interface which this logical device
		represents.

		Valid only if the device is a USB HID device.
		Set to -1 in all other cases.
	*/
	int interface_number;

	/** Pointer to the next device */
	struct hid_device_info* next;

	/** Underlying bus type
		Since version 0.13.0, @ref HID_API_VERSION >= HID_API_MAKE_VERSION(0, 13, 0)
	*/
	hid_bus_type bus_type;
};

/* PAR@@@: Not really needed but anyway */
#define HID_API_EXPORT /**< API export macro */
#define HID_API_CALL /**< API call macro */

/*	https://wicg.github.io/webhid/#requestdevice-method
	https://wicg.github.io/webhid/#dom-hid-getdevices
	https://web.dev/hid/
	https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#embind-val-guide
	https://emscripten.org/docs/api_reference/val.h.html
	https://web.dev/articles/emscripten-embedding-js-snippets
	https://emscripten.org/docs/porting/asyncify.html
	https://emscripten.org/docs/tools_reference/emcc.html#emcc-s-option-value */

struct hid_device_info HID_API_EXPORT* HID_API_CALL hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
//	val device = val::global("Navigator.hid").call<val>("requestDevice", std::string("{filters: [{ vendorId: 0x046d }, { vendorId: 0x256f }]}"));
//	val webhid = val::global("Navigator.hid");
	val::global("console").call<void>("log", std::string("hid_enumerate"));
	val navigator = val::global("navigator");
	val hid = navigator["hid"];
	if (hid.isUndefined()) {
//	if (!hid.as<bool>()) {
		printf("WebHID API not supported. Check https://caniuse.com/webhid for a list of browsers that support it.\n");
		return 0;
	} else {
//		val hid = webhid.new_();
//		val device = hid.call<void>("requestDevice", "{filters: [{ vendorId: 0x046d }, { vendorId: 0x256f }]}");
		//val reqd = hid["requestDevice"].await();
//		val devices = hid.call<val>("requestDevice", std::string("{filters: [{ vendorId: 0x046d }, { vendorId: 0x256f }]}")).await();


		// Define the filters
		val filters = val::array();
		val filter1 = val::object();
		filter1.set("vendorId", "0x046d");
		filters.call<void>("push", filter1);
		val filter2 = val::object();
		filter2.set("vendorId", "0x256f");
		filters.call<void>("push", filter2);

		// Create options object with filters
		val options = val::object();
		options.set("filters", filters);

		val::global("console").call<void>("log", options);

		val devices = hid.call<val>("requestDevice", options).await();

		val::global("console").call<void>("log", devices);
//		std::cout << "Devices: " << devices.as<std::string>() << std::endl;
		if (0 == devices["length"].as<size_t>()) {
			return 0;
		}
		else {
			hid_device_info* devs = new hid_device_info;
			devs->path = (char*)(new val(devices[0]));
			devs->product_string = (wchar_t*)calloc(devices[0]["productName"].as<std::string>().length(), sizeof(char));
			wcscpy(devs->product_string, (wchar_t*)(devices[0]["productName"].as<std::string>().c_str()));
			devs->product_id = devices[0]["productId"].as<unsigned short>();
			devs->vendor_id = devices[0]["vendorId"].as<unsigned short>();

			return devs;
		}
	}
}

void  HID_API_EXPORT HID_API_CALL hid_free_enumeration(struct hid_device_info* devs)
{
	val::global("console").call<void>("log", std::string("hid_free_enumeration"));

	while (devs) {
		hid_device_info* current = devs;
		devs = devs->next;
		delete current->path;
		delete current->serial_number;
		delete current->manufacturer_string;
		delete current->product_string;
		delete current;
	}
}

static void hid_handleInputReport(val e) {
	val::global("console").call<void>("log", std::string("hid_handleInputReport"));
	val::global("console").call<void>("log", e);
	e.call<void>("preventDefault");				// don't interfere with regular mouse cursor
	e.call<void>("stopImmediatePropagation");	// don't interfere with regular mouse cursor

}

EMSCRIPTEN_BINDINGS(my_module) {
	function("hid_handleInputReport", &hid_handleInputReport);
}

HID_API_EXPORT hid_device* HID_API_CALL hid_open_path(const char* path)
{
	val::global("console").call<void>("log", std::string("hid_open_path"));

	val* device = (val *)path;
	val::global("console").call<void>("log", *device);
	device->call<val>("open").await();
	val::global("console").call<void>("log", *device);
	if (true == (*device)["opened"].as<bool>()) {
		device->call<void>("addEventListener", std::string("inputreport"), val::module_property("hid_handleInputReport"));
/*		device->call<void>("addEventListener", "inputreport", function(val event) {
			var data = new Uint8Array(event.data.buffer);
			Module._cppInputReportHandler(data.byteOffset, data.byteLength);
		});*/
		return (hid_device*)path;
	}

	return 0;
}

int  HID_API_EXPORT HID_API_CALL hid_write(hid_device* dev, const unsigned char* data, size_t length)
{
	val::global("console").call<void>("log", std::string("hid_write"));

	return -1;
}

int  HID_API_EXPORT HID_API_CALL hid_read(hid_device* dev, unsigned char* data, size_t length)
{
	val::global("console").call<void>("log", std::string("hid_read"));

	return -1;
}

int  HID_API_EXPORT HID_API_CALL hid_set_nonblocking(hid_device* dev, int nonblock)
{
	val::global("console").call<void>("log", std::string("hid_set_nonblocking"));

	return 0;
}

int HID_API_EXPORT HID_API_CALL hid_send_feature_report(hid_device* dev, const unsigned char* data, size_t length)
{
	val::global("console").call<void>("log", std::string("hid_send_feature_report"));

	return -1;
}

void HID_API_EXPORT HID_API_CALL hid_close(hid_device* dev)
{
	val::global("console").call<void>("log", std::string("hid_close"));

}
