#include "idevice/service/amfiservice.h"

#include <functional>  // std::bind, std::placeholders

#include "idevice/common/macro_def.h"
#include "plist/plist.h"

using namespace idevice;

constexpr char kAMFIServiceName[] = "com.apple.amfi.lockdown";

IService::Result AMFIService::NewClient(idevice_t device, lockdownd_service_descriptor_t service,
                                        AMFIClient* client) {
  property_list_service_client_t service_client = nullptr;
  property_list_service_error_t err =
      property_list_service_client_new(device, service, &service_client);
  if (err != kResultOk) {
    return err;
  }

  *client = service_client;
  return ResultCode::kOk;
}

IService::Result AMFIService::Connect(idevice_t device) {
  int32_t err = 0;

  service_error_t ret =
      service_client_factory_start_service(device, kAMFIServiceName, (void**)&client_, kClientLabel,
                                           SERVICE_CONSTRUCTOR(AMFIService::NewClient), &err);
  IDEVICE_LOG_D("service_client_factory_start_service, name=%s, ret=%d, err=%d\n", kAMFIServiceName,
                ret, err);
  return ret;
}

IService::Result AMFIService::Disconnect() {
  if (client_ != nullptr) {
    property_list_service_client_free(client_);
    client_ = nullptr;
  }
  return LOCKDOWN_E_SUCCESS;
}

IService::Result AMFIService::SetDeveloperMode(int mode) {
  plist_t dict = plist_new_dict();
  plist_dict_set_item(dict, "action", plist_new_uint(mode));

  property_list_service_error_t ret = property_list_service_send_xml_plist(client_, dict);
  if (ret != PROPERTY_LIST_SERVICE_E_SUCCESS) {
    plist_free(dict);
    return ret;
  }
  plist_free(dict);

  dict = plist_new_dict();
  ret = property_list_service_receive_plist(client_, &dict);
  return ret;
}
