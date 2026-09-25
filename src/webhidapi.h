/*
 * Minimal hidapi-style wrapper for WebHID, used by the Emscripten build.
 */
#ifndef WEBHIDAPI_H_
#define WEBHIDAPI_H_

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <map>
#include <queue>
#include <string>
#include <vector>

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

#define HID_API_EXPORT
#define HID_API_CALL

typedef void (hid_input_report_callback)(unsigned char* data, size_t length, void* uptr);

struct hid_device_ {
	val* js_device;
	hid_input_report_callback* input_report_callback;
	void* input_report_user_data;
	int nonblocking;
	std::queue<std::vector<unsigned char> > reports;
};

static std::map<std::string, val*> webhid_device_registry;
static unsigned long webhid_next_path_id = 1;

void HID_API_EXPORT HID_API_CALL hid_free_enumeration(struct hid_device_info* devs);

static char* webhid_strdup(const std::string& str)
{
	size_t len = str.size() + 1;
	char* copy = (char*)malloc(len);

	if(copy) {
		memcpy(copy, str.c_str(), len);
	}
	return copy;
}

static wchar_t* webhid_wcsdup_ascii(const std::string& str)
{
	size_t len = str.size();
	wchar_t* copy = (wchar_t*)calloc(len + 1, sizeof(wchar_t));

	if(copy) {
		for(size_t i = 0; i < len; ++i) {
			copy[i] = (unsigned char)str[i];
		}
	}
	return copy;
}

static std::string webhid_get_string_prop(const val& obj, const char* prop)
{
	val v = obj[prop];

	if(v.isUndefined() || v.isNull()) {
		return std::string();
	}
	return v.as<std::string>();
}

static val webhid_build_filters(unsigned short vendor_id, unsigned short product_id)
{
	val filters = val::array();

	if(vendor_id) {
		val filter = val::object();
		filter.set("vendorId", vendor_id);
		if(product_id) {
			filter.set("productId", product_id);
		}
		filters.call<void>("push", filter);
	} else {
		static const unsigned short vendors[] = {0x046d, 0x256f};
		for(size_t i = 0; i < sizeof vendors / sizeof vendors[0]; ++i) {
			val filter = val::object();
			filter.set("vendorId", vendors[i]);
			filters.call<void>("push", filter);
		}
	}

	return filters;
}

static struct hid_device_info* webhid_append_device_info(struct hid_device_info** tail, const val& device)
{
	struct hid_device_info* info;
	std::string path_key;
	std::string manufacturer;
	std::string product;
	val usage_page;
	val usage;

	if(!(info = (struct hid_device_info*)calloc(1, sizeof *info))) {
		return 0;
	}

	path_key = std::string("webhid:") + std::to_string(webhid_next_path_id++);
	webhid_device_registry[path_key] = new val(device);

	info->path = webhid_strdup(path_key);
	info->vendor_id = device["vendorId"].as<unsigned short>();
	info->product_id = device["productId"].as<unsigned short>();
	manufacturer = webhid_get_string_prop(device, "manufacturerName");
	product = webhid_get_string_prop(device, "productName");
	info->manufacturer_string = webhid_wcsdup_ascii(manufacturer);
	info->product_string = webhid_wcsdup_ascii(product);
	info->interface_number = -1;
	info->bus_type = HID_API_BUS_USB;
	usage_page = device["usagePage"];
	usage = device["usage"];
	if(!usage_page.isUndefined() && !usage_page.isNull()) {
		info->usage_page = usage_page.as<unsigned short>();
	}
	if(!usage.isUndefined() && !usage.isNull()) {
		info->usage = usage.as<unsigned short>();
	}

	(*tail)->next = info;
	*tail = info;
	return info;
}

struct hid_device_info HID_API_EXPORT* HID_API_CALL hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
	struct hid_device_info head;
	struct hid_device_info* tail = &head;
	val navigator = val::global("navigator");
	val hid = navigator["hid"];
	val devices;
	val options;
	size_t count;

	memset(&head, 0, sizeof head);

	if(hid.isUndefined() || hid.isNull()) {
		printf("WebHID API not supported. Check https://caniuse.com/webhid for a list of browsers that support it.\n");
		return 0;
	}

	options = val::object();
	options.set("filters", webhid_build_filters(vendor_id, product_id));
	devices = hid.call<val>("requestDevice", options).await();
	count = devices["length"].as<size_t>();

	for(size_t i = 0; i < count; ++i) {
		if(!webhid_append_device_info(&tail, devices[i])) {
			hid_free_enumeration(head.next);
			return 0;
		}
	}

	return head.next;
}

void HID_API_EXPORT HID_API_CALL hid_free_enumeration(struct hid_device_info* devs)
{
	while(devs) {
		struct hid_device_info* current = devs;

		devs = devs->next;
		free(current->path);
		free(current->serial_number);
		free(current->manufacturer_string);
		free(current->product_string);
		free(current);
	}
}

static void hid_handleInputReport(val event)
{
	val target = event["currentTarget"];
	val handle_value = target["__spnav_handle"];
	hid_device* handle;
	val data_view;
	size_t length;
	std::vector<unsigned char> report;

	if(handle_value.isUndefined() || handle_value.isNull()) {
		return;
	}

	handle = (hid_device*)handle_value.as<uintptr_t>();
	if(!handle) {
		return;
	}

	data_view = event["data"];
	length = data_view["byteLength"].as<size_t>();
	report.resize(length + 1);
	report[0] = event["reportId"].as<unsigned char>();

	for(size_t i = 0; i < length; ++i) {
		report[i + 1] = data_view.call<unsigned char>("getUint8", (unsigned)i);
	}

	if(handle->input_report_callback) {
		handle->input_report_callback(report.data(), report.size(), handle->input_report_user_data);
	} else {
		handle->reports.push(report);
	}
}

EMSCRIPTEN_BINDINGS(webhidapi_module) {
	function("hid_handleInputReport", &hid_handleInputReport);
}

HID_API_EXPORT hid_device* HID_API_CALL hid_open_path(const char* path)
{
	std::map<std::string, val*>::const_iterator it;
	hid_device* handle;

	if(!path) {
		return 0;
	}

	it = webhid_device_registry.find(path);
	if(it == webhid_device_registry.end()) {
		return 0;
	}

	handle = new hid_device();
	handle->js_device = new val(*it->second);
	handle->input_report_callback = 0;
	handle->input_report_user_data = 0;
	handle->nonblocking = 1;

	handle->js_device->call<val>("open").await();
	if(!(*handle->js_device)["opened"].as<bool>()) {
		delete handle->js_device;
		delete handle;
		return 0;
	}

	handle->js_device->set("__spnav_handle", val((uintptr_t)handle));
	handle->js_device->call<void>("addEventListener", std::string("inputreport"), val::module_property("hid_handleInputReport"));
	return handle;
}

HID_API_EXPORT int HID_API_CALL hid_set_callback(hid_device* dev,
		hid_input_report_callback input_report_callback, void* uptr)
{
	if(!dev || !dev->js_device || !(*dev->js_device)["opened"].as<bool>()) {
		return -1;
	}

	dev->input_report_callback = input_report_callback;
	dev->input_report_user_data = uptr;
	return 0;
}

int HID_API_EXPORT HID_API_CALL hid_write(hid_device* dev, const unsigned char* data, size_t length)
{
	if(!dev || !dev->js_device || !data || length == 0) {
		return -1;
	}

	dev->js_device->call<val>("sendReport", (unsigned)data[0],
			val(typed_memory_view(length - 1, data + 1))).await();
	return (int)length;
}

int HID_API_EXPORT HID_API_CALL hid_read(hid_device* dev, unsigned char* data, size_t length)
{
	std::vector<unsigned char> report;
	size_t copy_len;

	if(!dev || !data || !length) {
		return -1;
	}

	if(dev->reports.empty()) {
		return 0;
	}

	report = dev->reports.front();
	dev->reports.pop();
	copy_len = length < report.size() ? length : report.size();
	memcpy(data, report.data(), copy_len);
	return (int)copy_len;
}

int HID_API_EXPORT HID_API_CALL hid_set_nonblocking(hid_device* dev, int nonblock)
{
	if(!dev) {
		return -1;
	}

	dev->nonblocking = nonblock;
	return 0;
}

int HID_API_EXPORT HID_API_CALL hid_send_feature_report(hid_device* dev, const unsigned char* data, size_t length)
{
	if(!dev || !dev->js_device || !data || length == 0) {
		return -1;
	}

	dev->js_device->call<val>("sendFeatureReport", (unsigned)data[0],
			val(typed_memory_view(length - 1, data + 1))).await();
	return (int)length;
}

void HID_API_EXPORT HID_API_CALL hid_close(hid_device* dev)
{
	if(!dev) {
		return;
	}

	if(dev->js_device) {
		if((*dev->js_device)["opened"].as<bool>()) {
			dev->js_device->call<void>("removeEventListener", std::string("inputreport"),
					val::module_property("hid_handleInputReport"));
			dev->js_device->call<val>("close").await();
		}
		delete dev->js_device;
	}

	delete dev;
}

#endif	/* WEBHIDAPI_H_ */
