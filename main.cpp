#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>
#include <algorithm>
#include <Shlwapi.h>
#include <fstream>
#include <map>

using namespace std;

struct fileInfo {
	string path;
	string name;
};

/* UTILITY */
vector<fileInfo> filesindir(string);
vector<fileInfo> filesindir_recursive(string);
string getCwd();
void help();
void findImagesInFiles(string);
string stripCurrentPath(string);

string currentPath = getCwd();
string imageTypes[] = {"jpg","jpeg","gif","png","svg","bmp","tif","svg"};
string textTypes[] = {"htm","html","xml","php","asp","jsp","txt","php3","js","css","css3","inc","sql","local","tpl","less","yml"};
string ext;
vector<fileInfo> unusuedImages;
vector<fileInfo> images;
string type = "";

map<string,string> result;

int main(int argc, char* argv[]) {

	string imagesPath = "";
	string imagePath = "";
	string textSingleImage = "";
	vector<fileInfo> files;
	int ok = 0;

	if (argv[1]) {
		if (string(argv[1])=="-help") {

			help();

			return 1;

		} else if (PathIsDirectory(argv[1])) {
			imagesPath=argv[1];

			files = filesindir_recursive(currentPath+"\\"+imagesPath);
			for (unsigned int i=0; i < files.size(); i++) {
				ext = files[i].name.substr(files[i].name.find_last_of(".") + 1);
				ok = 0;
				for(auto &t: imageTypes) if (ext==t) ok=1;
				if (ok) images.push_back(files[i]);
			}

		} else {
			imagePath=argv[1];
			string filenamepart=imagePath.substr(imagePath.find_last_of("\\") + 1);
			int pos = imagePath.find(filenamepart,0);
			string filepathpart=imagePath.replace(pos, filenamepart.size()+1, "");
			images.push_back({filepathpart,filenamepart});

			textSingleImage=" di " + string(argv[1]);
		}
	} else {
		cout << "Non va bene, devi specificare una directory o un file immagine." << endl << endl;
		help();
		return 1;
	}

	if (argv[2]&&argv[3]) {
		if (string(argv[2])=="-type") {
			type=argv[3];
		}
	}

	if (type!="")
		cout << "> Inizio la ricerca"<<textSingleImage<<" nei file ." << type << " in " << currentPath << endl << endl;
	else
		cout << "> Inizio la ricerca"<<textSingleImage<<" in " << currentPath << endl << endl;

	/*for(auto &imm: images) {
		cout << imm.path << "\\" << imm.name << endl;
	}*/

	findImagesInFiles(currentPath);

	for(auto &imm: images) {
		if (!result.count(imm.name)) unusuedImages.push_back(imm);
	}

	if (result.size()>0) {
		cout << endl << "> Immagini trovate nel percorso corrente:" << endl << endl;
		for(auto &r: result) {
			cout << " - " << r.first << " (" << stripCurrentPath(r.second) << ")" <<endl;
		}
	} else {
		cout << endl << "> Nessuna immagine trovata" << endl;
		return 0;
	}

	if (unusuedImages.size()>0) {
		cout << endl << "> Immagini inutilizzate:" << endl << endl;
		for(auto &unusued: unusuedImages) {
			cout << " - " << stripCurrentPath(unusued.path) << "\\" << unusued.name << endl;
		}
	} else {
		cout << endl << "> Nessuna immagine inutilizzata" << endl;
	}
    return 0;
}

// Utility functions

vector<fileInfo> filesindir(string path) {
	vector<fileInfo> files;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir (path.c_str())) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			files.push_back({path,ent->d_name});
		}
		closedir (dir);
	}
	return files;
}

vector<fileInfo> filesindir_recursive(string path) {
	vector<fileInfo> resultfiles;
	vector<fileInfo> rootfiles;
	vector<fileInfo> dirfiles;
	string fpath;

	rootfiles = filesindir(path);
	resultfiles.insert(resultfiles.end(), rootfiles.begin(), rootfiles.end());

	for (unsigned int i=0; i < rootfiles.size(); i++) {
		fpath=path+"\\"+rootfiles[i].name;
		if (rootfiles[i].name=="."||rootfiles[i].name=="..") continue;
		if (PathIsDirectory(fpath.c_str())) {
			dirfiles=filesindir_recursive(fpath);
			resultfiles.insert(resultfiles.end(), dirfiles.begin(), dirfiles.end());
		}
	}
	return resultfiles;
}

string getCwd() {
	char buff[PATH_MAX];
	getcwd(buff, PATH_MAX);
	string c(buff);
	return c;
}

string stripCurrentPath(string p) {
	return p.replace(0, currentPath.size()+1, "");
}

void help(){
	string iim="";
	string ifi="";
	for (auto &e : imageTypes) iim+=","+e;
	for (auto &e : textTypes) ifi+=","+e;

	iim.erase(0, 1);
	ifi.erase(0, 1);

	cout << "USAGE: " << endl;
	cout << " - obsim <subdir> : search images in <subdir> in files (recursive)" << endl;
	cout << " - obsim <subdir> -type php : search only in *.php files" << endl;
	cout << " - obsim <imagefile> : search for <imagefile> in files (recursive)" << endl;
	cout << "SEARCH IMAGE TYPES: " << iim << endl;
	cout << "SEARCH IN FILE TYPES: " << ifi << endl;
}

void findImagesInFiles(string path) {

	ifstream cfile;
	vector<string> resultFiles;
	vector<fileInfo> sub;
	string fpath;
	string fname;
	bool ok = false;
	string ext;

	vector<fileInfo> pfiles = filesindir(path);

	for (unsigned int i=0; i < pfiles.size(); i++) {

		fname=pfiles[i].name;
		fpath=pfiles[i].path+"\\"+fname;

		if (fname=="."||fname=="..") continue;

		if (PathIsDirectory(fpath.c_str())) {
			findImagesInFiles(fpath);
			continue;
		}

		ok = false;
		ext = fname.substr(fname.find_last_of(".") + 1);

		if (type!="") {
			ok = (ext==type);
		} else {
			for(auto &t: textTypes) if (ext==t) ok=true;
		}

		if (!ok) continue;

		cfile.open(fpath);

		if (!cfile.is_open()) continue;

		cout << " - Cerco in " << stripCurrentPath(fpath) << endl;

		string line;

		while(getline(cfile, line)) {
			if (line.size()==0) continue;
			for(auto &imm: images) {
				if (line.find(imm.name) != std::string::npos) {
					result[imm.name]=fpath;
					continue;
				}
			}
		}
		cfile.close();
	}
}
