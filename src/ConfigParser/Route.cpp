#include "../../include/Config.hpp"
#include "../../include/Route.hpp"


std::string ft_trim(const std::string& s) 
{
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end   = s.find_last_not_of(" \t\n\r");

    if (start == std::string::npos)
        throw std::runtime_error("empty parameter");

    return s.substr(start, end - start + 1);
}

Route::Route(std::string line)
{
    std::vector<std::string> params;
    std::string word;
    std::vector<std::pair<std::string, std::string> > pairs;

    std::replace(line.begin(), line.end(), ':', ',');
    std::stringstream ss(line);
    
    while (std::getline(ss, word, ','))
        params.push_back(word);
    
    for (size_t i = 0; i < params.size(); i++)
    {
        std::stringstream param_ss(params[i]);
        std::string key, value;
        if (std::getline(param_ss, key, '=') && std::getline(param_ss, value))
        {
            key = ft_trim(key);
            value = ft_trim(value);
            pairs.push_back(std::make_pair(key, value));
        }
        if (key.empty() || value.empty())
            throw std::runtime_error("key or value is empty");
    }

    for (size_t i = 0; i < pairs.size(); i++)
    {
        if (pairs[i].first == "route")
            insertName(pairs[i].second);
        else if (pairs[i].first == "ROOT")
            insertRootPath(pairs[i].second);
        else if (pairs[i].first == "DEFAULT_FILE")
            insertDefaultFile(pairs[i].second);
        else if (pairs[i].first == "ALLOWED_METHODS")
            insertAllowedMethods(pairs[i].second);
        else if (pairs[i].first == "UPLOAD_DIR")
            insertUploadDir(pairs[i].second);
        else if (pairs[i].first == "CGI_EXTENTION")
            insertCgiExtention(pairs[i].second);
        else if (pairs[i].first == "DIR_LISTING")
            insertDirListing(pairs[i].second);
        else if (pairs[i].first == "REDIRECT")
            insertRedirect(pairs[i].second);
        else
            throw std::runtime_error("invalid configuration key");
    }
}

void Route::insertName(std::string &name)
{
    if (name[0] != '/')
        throw std::runtime_error("route name should start with '/' ");
    this->name = name;
}
void Route::insertRootPath(std::string &rootPath)
{
    struct stat info;
    if (stat(rootPath.c_str(), &info) != 0)
        throw std::runtime_error("RootPath not valid");
    this->rootPath = rootPath;
}
void Route::insertDefaultFile(std::string &defaultFile)
{
    int i = defaultFile.length();
    if (i < 5 || defaultFile[i - 1] != 'l' || defaultFile[i - 2] != 'm' || defaultFile[i - 3] != 't' || defaultFile[i - 4] != 'h' || defaultFile[0] == '.')
        throw std::runtime_error("default file syntax not valid");
    this->default_file = defaultFile;
}
void Route::insertAllowedMethods(std::string &allowedMethods)
{
    // get-post-delete
    std::string word;
    std::stringstream ss(allowedMethods);
    while (std::getline(ss, word, '-'))
    {
        if (word != "POST" && word != "DELETE" && word != "GET")
            throw std::runtime_error(word + " methods not valid");
        else
            this->allowed_methods.push_back(word);
    }
}
void Route::insertUploadDir(std::string &uploadDir)
{
    struct stat info1;
    if (stat(uploadDir.c_str(), &info1) != 0)
        throw std::runtime_error("UploadDir Path not valid");
    this->upload_dir = uploadDir;
}

bool validCgiExtension(std::string& ext) {
    const char* cgi_extensions[] = { ".cgi", ".pl", ".py", ".php", ".asp",
                                    ".shtml", ".fcgi", ".sh", ".plx", ".jsp", ".rb", ".xml" };

    for (int i = 0; i < 12; i++) {
        if (ext == cgi_extensions[i])
            return true;
    }
    return false;
}

void Route::insertCgiExtention(std::string &cgiExtention)
{
    std::stringstream ss(cgiExtention);
    std::string word;
    while (std::getline(ss, word, '-'))
    {
        word = ft_trim(word);
        if (!validCgiExtension(word))
            throw std::runtime_error("CGI_EXTENTION not valid");
        this->cgi_extention.push_back(word);
    }
}
void Route::insertDirListing(std::string &dirListing)
{
    if (dirListing != "on" && dirListing != "off")
        throw std::runtime_error("dirListing not valid");
    this->dir_listing = dirListing;
}

bool isNumber(const std::string& s) 
{
    if (s.empty())
        return false;

    for (size_t i = 0; i < s.size(); ++i) {
        if (!std::isdigit(s[i]))
            return false;
    }
    return true;
}
void Route::insertRedirect(std::string &redirect)
{
    std::stringstream ss(redirect);
    std::string word;
    std::string signal;
    int num;
    if (std::getline(ss, signal, '-') && std::getline(ss, word))
    {
        signal = ft_trim(signal);
        word = ft_trim(word);
        if (std::count(word.begin(), word.end(), '.') > 1)
            word = "/https:/" + word;
        if (!isNumber(signal) || word[0] != '/')
            throw std::runtime_error("redirect not valid");
        num = std::atoi(signal.c_str());
        word.erase(0, 1);
        this->redirect.insert(std::make_pair(num, word));
    }
}
