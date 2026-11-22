#include <httplib.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
using json = nlohmann::json;
namespace fs = std::filesystem;
static const fs::path DATA_FILE = fs::path("..") / "data" / "students.json";
static std::mutex dataMutex;
json loadStudents() {
  std::lock_guard<std::mutex> lock(dataMutex);
  if (!fs::exists(DATA_FILE)) { fs::create_directories(DATA_FILE.parent_path()); std::ofstream(DATA_FILE) << "[]"; }
  std::ifstream in(DATA_FILE); if (!in) return json::array();
  try { json value; in >> value; return value.is_array() ? value : json::array(); } catch (...) { return json::array(); }
}
bool saveStudents(const json& students) {
  std::lock_guard<std::mutex> lock(dataMutex); fs::create_directories(DATA_FILE.parent_path());
  std::ofstream out(DATA_FILE); if (!out) return false; out << students.dump(2); return bool(out);
}
bool hasText(const json& value,const char* key) { return value.contains(key)&&value[key].is_string()&&!value[key].get<std::string>().empty(); }
int main() {
  httplib::Server server;
  server.set_default_headers({{"Access-Control-Allow-Origin","*"},{"Access-Control-Allow-Headers","Content-Type"},{"Access-Control-Allow-Methods","GET, POST, PUT, DELETE, OPTIONS"}});
  server.Options(R"(/api/.*)",[](const httplib::Request&,httplib::Response& res){res.status=204;});
  server.Get("/api/health",[](const httplib::Request&,httplib::Response& res){res.set_content(R"({"status":"ok"})","application/json");});
  server.Get("/api/students",[](const httplib::Request&,httplib::Response& res){res.set_content(loadStudents().dump(),"application/json");});
  server.Post("/api/students",[](const httplib::Request& req,httplib::Response& res){
    json body; try{body=json::parse(req.body);}catch(...){res.status=400;res.set_content(R"({"error":"Invalid JSON"})","application/json");return;}
    for(const char* key:{"name","rollNumber","email","department","year"}) if(!hasText(body,key)){res.status=400;res.set_content(json{{"error",std::string("Missing required field: ")+key}}.dump(),"application/json");return;}
    json students=loadStudents();
    auto duplicate=std::find_if(students.begin(),students.end(),[&](const json& s){return s.value("rollNumber","")==body.value("rollNumber","");});
    if(duplicate!=students.end()){res.status=409;res.set_content(R"({"error":"Roll number already exists"})","application/json");return;}
    body["id"]="STU-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()%100000000);
    body["createdAt"]=std::chrono::system_clock::now().time_since_epoch().count(); body["phone"]=body.value("phone","");
    students.push_back(body); if(!saveStudents(students)){res.status=500;return;} res.status=201;res.set_content(body.dump(),"application/json");
  });
  server.Put(R"(/api/students/([A-Za-z0-9-]+))",[](const httplib::Request& req,httplib::Response& res){
    json body;try{body=json::parse(req.body);}catch(...){res.status=400;return;} json students=loadStudents();std::string id=req.matches[1];
    auto it=std::find_if(students.begin(),students.end(),[&](const json& s){return s.value("id","")==id;});
    if(it==students.end()){res.status=404;return;}
    for(const char* key:{"name","rollNumber","email","department","year","phone"})if(body.contains(key))(*it)[key]=body[key];
    if(!saveStudents(students)){res.status=500;return;}res.set_content(it->dump(),"application/json");
  });
  server.Delete(R"(/api/students/([A-Za-z0-9-]+))",[](const httplib::Request& req,httplib::Response& res){
    json students=loadStudents();std::string id=req.matches[1];auto before=students.size();
    students.erase(std::remove_if(students.begin(),students.end(),[&](const json& s){return s.value("id","")==id;}),students.end());
    if(students.size()==before){res.status=404;return;}if(!saveStudents(students)){res.status=500;return;}res.set_content(R"({"success":true})","application/json");
  });
  std::cout<<"Student API running at http://localhost:8080\n";server.listen("0.0.0.0",8080);
}
