#include "unit_store.h"
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <core/unit.h>

std::shared_ptr<ambr::store::UnitStore> ambr::store::UnitStore::CreateUnitStoreByBytes(const std::vector<uint8_t> &buf){
  std::shared_ptr<ambr::core::Unit> unit = core::Unit::CreateUnitByByte(buf);
  if(!unit)return std::shared_ptr<store::UnitStore>();
  if(unit->type() == core::UnitType::send){
    std::shared_ptr<core::SendUnit> send_unit = std::dynamic_pointer_cast<core::SendUnit>(unit);
    return std::make_shared<store::SendUnitStore>(send_unit);
  }else if(unit->type() == core::UnitType::receive){
    std::shared_ptr<core::ReceiveUnit> receive_unit = std::dynamic_pointer_cast<core::ReceiveUnit>(unit);
    return std::make_shared<store::ReceiveUnitStore>(receive_unit);
  }else{
    return std::shared_ptr<store::UnitStore>();
  }
}

ambr::store::SendUnitStore::SendUnitStore(std::shared_ptr<core::SendUnit> unit):
  UnitStore(ST_SendUnit),
  unit_(unit),
  version_(0x00000001),
  is_validate_(false){
  //assert(unit);
}

std::shared_ptr<ambr::core::SendUnit> ambr::store::SendUnitStore::unit(){
  return unit_;
}

ambr::core::UnitHash ambr::store::SendUnitStore::receive_unit_hash() const{
  return receive_unit_hash_;
}

void ambr::store::SendUnitStore::set_receive_unit_hash(const ambr::core::UnitHash hash){
  receive_unit_hash_ = hash;
}

uint32_t ambr::store::SendUnitStore::version(){
  return version_;
}

void ambr::store::SendUnitStore::set_version(uint32_t version){
  version_ = version;
}

uint8_t ambr::store::SendUnitStore::is_validate(){
  return is_validate_;
}

void ambr::store::SendUnitStore::set_is_validate(uint8_t validate){
  is_validate_ = validate;
}

std::string ambr::store::SendUnitStore::SerializeJson() const{
  assert(unit_);
  boost::property_tree::ptree pt;
  boost::property_tree::ptree store_pt;
  std::stringstream stream(unit_->SerializeJson());
  try{
    boost::property_tree::read_json(stream, pt);
    store_pt.put("version", version_);
    store_pt.put("receive_unit_hash", receive_unit_hash_.encode_to_hex());
    store_pt.put("is_validate", is_validate_);
    pt.add_child("store_addtion", store_pt);
  }catch(...){
    assert(1);
  }
  std::ostringstream out_stream;
  boost::property_tree::write_json(out_stream, pt);
  return out_stream.str();
}

bool ambr::store::SendUnitStore::DeSerializeJson(const std::string &json){
  boost::property_tree ::ptree pt;
  std::stringstream stream(json);
  boost::property_tree::read_json(stream, pt);
  try{
    version_ = pt.get<uint32_t>("store_addtion.version");
    if(version_ == 0x00000001){
      receive_unit_hash_.decode_from_hex(pt.get<std::string>("store_addtion.receive_unit_hash"));
      is_validate_ = pt.get<uint8_t>("store_addtion.is_validate");
    }else{
      return false;
    }
    unit_ = std::make_shared<core::SendUnit>();
    if(!unit_->DeSerializeJson(json)){
      return false;
    }
  }catch(...){
    return false;
  }
  return true;
}

std::vector<uint8_t> ambr::store::SendUnitStore::SerializeByte() const{
  assert(unit_);
  std::vector<uint8_t> rtn = unit_->SerializeByte();
  rtn.insert(rtn.end(), (uint8_t*)&version_, (uint8_t*)&version_+sizeof(version_));
  rtn.insert(rtn.end(),receive_unit_hash_.bytes().begin(), receive_unit_hash_.bytes().end());
  rtn.push_back(is_validate_);
  return rtn;
}

bool ambr::store::SendUnitStore::DeSerializeByte(const std::vector<uint8_t> &buf){
  unit_ = std::make_shared<core::SendUnit>();
  size_t used_count = 0;
  if(!unit_->DeSerializeByte(buf, &used_count)){
    return false;
  }
  if(buf.size()-used_count < sizeof(version_)){
    return false;
  }
  const uint8_t* src = &buf[used_count];
  memcpy(&version_, src, sizeof(version_));
  src += sizeof(version_);
  if(version_ == 0x00000001){
    if(buf.size()-used_count < sizeof(version_)+sizeof(receive_unit_hash_)+sizeof(is_validate_)){
      return false;
    }
    memcpy(&receive_unit_hash_, src, sizeof(receive_unit_hash_));
    src += sizeof(receive_unit_hash_);
    memcpy(&is_validate_, src, sizeof(is_validate_));
    return true;
  }//else if(other version)
  return false;
}

std::shared_ptr<ambr::core::Unit> ambr::store::SendUnitStore::GetUnit(){
  return unit_;
}


ambr::store::ReceiveUnitStore::ReceiveUnitStore(std::shared_ptr<core::ReceiveUnit> unit):
  UnitStore(ST_ReceiveUnit),
  unit_(unit),
  version_(0x00000001),
  is_validate_(false){
  //assert(unit);
}

std::shared_ptr<ambr::core::ReceiveUnit> ambr::store::ReceiveUnitStore::unit(){
  return unit_;
}

uint32_t ambr::store::ReceiveUnitStore::version(){
  return version_;
}

void ambr::store::ReceiveUnitStore::set_version(uint32_t version){
  version_ = version;
}

uint8_t ambr::store::ReceiveUnitStore::is_validate(){
  return is_validate_;
}

void ambr::store::ReceiveUnitStore::set_is_validate(uint8_t validate){
  is_validate_ = validate;
}

std::string ambr::store::ReceiveUnitStore::SerializeJson() const{
  assert(unit_);
  boost::property_tree::ptree pt;
  boost::property_tree::ptree store_pt;
  std::stringstream stream(unit_->SerializeJson());
  try{
    boost::property_tree::read_json(stream, pt);
    store_pt.put("version", version_);
    store_pt.put("is_validate", is_validate_);
    pt.add_child("store_addtion", store_pt);
  }catch(...){
    assert(1);
  }
  std::ostringstream out_stream;
  boost::property_tree::write_json(out_stream, pt);
  return out_stream.str();
}

bool ambr::store::ReceiveUnitStore::DeSerializeJson(const std::string &json){
  boost::property_tree::ptree pt;
  std::stringstream stream(json);
  boost::property_tree::read_json(stream, pt);
  try{
    version_ = pt.get<uint32_t>("store_addtion.version");
    if(version_ == 0x00000001){
      //deserialize other addtion
      is_validate_ = pt.get<uint8_t>("store_addtion.is_validate");
    }
    unit_ = std::make_shared<core::ReceiveUnit>();
    if(!unit_->DeSerializeJson(json)){
      return false;
    }
  }catch(...){
    return false;
  }
  return true;
}

std::vector<uint8_t> ambr::store::ReceiveUnitStore::SerializeByte() const{
  assert(unit_);
  std::vector<uint8_t> rtn = unit_->SerializeByte();
  rtn.insert(rtn.end(), (uint8_t*)&version_, (uint8_t*)&version_+sizeof(version_));
  rtn.push_back(is_validate_);
  return rtn;
}

bool ambr::store::ReceiveUnitStore::DeSerializeByte(const std::vector<uint8_t> &buf){
  unit_ = std::make_shared<core::ReceiveUnit>();
  size_t used_count = 0;
  if(!unit_->DeSerializeByte(buf, &used_count)){
    return false;
  }
  if(buf.size()-used_count < sizeof(version_)){
    return false;
  }
  const uint8_t* src = &buf[used_count];
  memcpy(&version_, src, sizeof(version_));
  src += sizeof(version_);
  if(version_ == 0x00000001){
    if(buf.size()-used_count < sizeof(version_)+sizeof(is_validate_)){
      return false;
    }
    memcpy(&is_validate_, src, sizeof(is_validate_));
    //deserialize addtion
    return true;
  }//else if(other version)
  return false;
}

std::shared_ptr<ambr::core::Unit> ambr::store::ReceiveUnitStore::GetUnit(){
  return unit_;
}

ambr::store::EnterValidatorSetUnitStore::EnterValidatorSetUnitStore(std::shared_ptr<core::EnterValidateSetUint> unit):
  UnitStore(ST_EnterValidatorSet),
  unit_(unit),
  version_(0x00000001),
  is_validate_(false){
  //assert(unit);
}

std::shared_ptr<ambr::core::EnterValidateSetUint> ambr::store::EnterValidatorSetUnitStore::unit(){
  return unit_;
}

uint32_t ambr::store::EnterValidatorSetUnitStore::version(){
  return version_;
}

void ambr::store::EnterValidatorSetUnitStore::set_version(uint32_t version){
  version_ = version;
}

uint8_t ambr::store::EnterValidatorSetUnitStore::is_validate(){
  return is_validate_;
}

void ambr::store::EnterValidatorSetUnitStore::set_is_validate(uint8_t validate){
  is_validate_ = validate;
}

std::string ambr::store::EnterValidatorSetUnitStore::SerializeJson() const{
  assert(unit_);
  boost::property_tree::ptree pt;
  boost::property_tree::ptree store_pt;
  std::stringstream stream(unit_->SerializeJson());
  try{
    boost::property_tree::read_json(stream, pt);
    store_pt.put("version", version_);
    store_pt.put("is_validate", is_validate_);
    pt.add_child("store_addtion", store_pt);
  }catch(...){
    assert(1);
  }
  std::ostringstream out_stream;
  boost::property_tree::write_json(out_stream, pt);
  return out_stream.str();
}

bool ambr::store::EnterValidatorSetUnitStore::DeSerializeJson(const std::string &json){
  boost::property_tree::ptree pt;
  std::stringstream stream(json);
  boost::property_tree::read_json(stream, pt);
  try{
    version_ = pt.get<uint32_t>("store_addtion.version");
    if(version_ == 0x00000001){
      //deserialize other addtion
      is_validate_ = pt.get<uint8_t>("store_addtion.is_validate");
    }
    unit_ = std::make_shared<core::EnterValidateSetUint>();
    if(!unit_->DeSerializeJson(json)){
      return false;
    }
  }catch(...){
    return false;
  }
  return true;
}

std::vector<uint8_t> ambr::store::EnterValidatorSetUnitStore::SerializeByte() const{
  assert(unit_);
  std::vector<uint8_t> rtn = unit_->SerializeByte();
  rtn.insert(rtn.end(), (uint8_t*)&version_, (uint8_t*)&version_+sizeof(version_));
  rtn.push_back(is_validate_);
  return rtn;
}

bool ambr::store::EnterValidatorSetUnitStore::DeSerializeByte(const std::vector<uint8_t> &buf){
  unit_ = std::make_shared<core::EnterValidateSetUint>();
  size_t used_count = 0;
  if(!unit_->DeSerializeByte(buf, &used_count)){
    return false;
  }
  if(buf.size()-used_count < sizeof(version_)){
    return false;
  }
  const uint8_t* src = &buf[used_count];
  memcpy(&version_, src, sizeof(version_));
  src += sizeof(version_);
  if(version_ == 0x00000001){
    if(buf.size()-used_count < sizeof(version_)+sizeof(is_validate_)){
      return false;
    }
    memcpy(&is_validate_, src, sizeof(is_validate_));
    //deserialize addtion
    return true;
  }//else if(other version)
  return false;
}

std::shared_ptr<ambr::core::Unit> ambr::store::EnterValidatorSetUnitStore::GetUnit(){
  return unit_;
}

ambr::store::LeaveValidatorSetUnitStore::LeaveValidatorSetUnitStore(std::shared_ptr<core::LeaveValidateSetUint> unit):
  UnitStore(ST_LeaveValidatorSet),
  unit_(unit),
  version_(0x00000001),
  is_validate_(false){
  //assert(unit);
}

std::shared_ptr<ambr::core::LeaveValidateSetUint> ambr::store::LeaveValidatorSetUnitStore::unit(){
  return unit_;
}

uint32_t ambr::store::LeaveValidatorSetUnitStore::version(){
  return version_;
}

void ambr::store::LeaveValidatorSetUnitStore::set_version(uint32_t version){
  version_ = version;
}

uint8_t ambr::store::LeaveValidatorSetUnitStore::is_validate(){
  return is_validate_;
}

void ambr::store::LeaveValidatorSetUnitStore::set_is_validate(uint8_t validate){
  is_validate_ = validate;
}

std::string ambr::store::LeaveValidatorSetUnitStore::SerializeJson() const{
  assert(unit_);
  boost::property_tree::ptree pt;
  boost::property_tree::ptree store_pt;
  std::stringstream stream(unit_->SerializeJson());
  try{
    boost::property_tree::read_json(stream, pt);
    store_pt.put("version", version_);
    store_pt.put("is_validate", is_validate_);
    pt.add_child("store_addtion", store_pt);
  }catch(...){
    assert(1);
  }
  std::ostringstream out_stream;
  boost::property_tree::write_json(out_stream, pt);
  return out_stream.str();
}

bool ambr::store::LeaveValidatorSetUnitStore::DeSerializeJson(const std::string &json){
  boost::property_tree::ptree pt;
  std::stringstream stream(json);
  boost::property_tree::read_json(stream, pt);
  try{
    version_ = pt.get<uint32_t>("store_addtion.version");
    if(version_ == 0x00000001){
      //deserialize other addtion
      is_validate_ = pt.get<uint8_t>("store_addtion.is_validate");
    }
    unit_ = std::make_shared<core::LeaveValidateSetUint>();
    if(!unit_->DeSerializeJson(json)){
      return false;
    }
  }catch(...){
    return false;
  }
  return true;
}

std::vector<uint8_t> ambr::store::LeaveValidatorSetUnitStore::SerializeByte() const{
  assert(unit_);
  std::vector<uint8_t> rtn = unit_->SerializeByte();
  rtn.insert(rtn.end(), (uint8_t*)&version_, (uint8_t*)&version_+sizeof(version_));
  rtn.push_back(is_validate_);
  return rtn;
}

bool ambr::store::LeaveValidatorSetUnitStore::DeSerializeByte(const std::vector<uint8_t> &buf){
  unit_ = std::make_shared<core::LeaveValidateSetUint>();
  size_t used_count = 0;
  if(!unit_->DeSerializeByte(buf, &used_count)){
    return false;
  }
  if(buf.size()-used_count < sizeof(version_)){
    return false;
  }
  const uint8_t* src = &buf[used_count];
  memcpy(&version_, src, sizeof(version_));
  src += sizeof(version_);
  if(version_ == 0x00000001){
    if(buf.size()-used_count < sizeof(version_)+sizeof(is_validate_)){
      return false;
    }
    memcpy(&is_validate_, src, sizeof(is_validate_));
    //deserialize addtion
    return true;
  }//else if(other version)
  return false;
}

std::shared_ptr<ambr::core::Unit> ambr::store::LeaveValidatorSetUnitStore::GetUnit(){
  return unit_;
}
