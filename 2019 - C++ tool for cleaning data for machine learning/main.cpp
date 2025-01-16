#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

using namespace std;

//----------------------------------------------------------------------

static const size_t kBufferSize = 1000;
static const char *kTrainingDataFile = "\\earthquake\\train.csv";
static const char *kQuakeDataFile = "\\earthquake\\quake_";
static const char *kQuakeDataFileOrdered = "\\earthquake\\ordered\\quake_";
static const char *kQuakeDataFileOrdClean = "\\earthquake\\ordered_clean\\quake_";
static const char *kFrequenciesFile = "\\earthquake\\frequencies";
static const char *kSortedFreqFile = "\\earthquake\\sorted_frequencies";

static const char kSeparator = ',';
static const char kZero = '0';

static const size_t kQuakeCount = 17;

//----------------------------------------------------------------------

struct AcousticTime
{
  int acousticData;
  double timeToFailure;
};

//----------------------------------------------------------------------

void splitTrainingData()
{
  // prepare training data
  string buffer;
  buffer.resize(kBufferSize);
  FILE *trainingData = __nullptr;
  fopen_s(&trainingData, kTrainingDataFile, "rt");
  fgets(&buffer[0], kBufferSize, trainingData); // skip header

  FILE *quakeData = nullptr;
  string quakeDataFile = kQuakeDataFile;
  size_t quakeCount = 0;
  size_t dataCount = 0;
  string quakeCountStr;

  size_t count = 0;
  size_t sepPos = 0;
  string acousticData;
  double timeToFailure = 1;
  double lastTimeToFailure = 2;
  vector<string> data;
  while (fgets(&buffer[0], kBufferSize, trainingData))
  {
    sepPos = buffer.find(kSeparator);
    if (sepPos == string::npos)
      continue;

    timeToFailure = stod(buffer.substr(sepPos + 1));
    if (timeToFailure > lastTimeToFailure)
    {
      // quake happened, save the data in separate file
      quakeCountStr = to_string(quakeCount);
      quakeDataFile = kQuakeDataFile;
      if (quakeCountStr.size() == 1)
        quakeDataFile.push_back(kZero);
      quakeDataFile += quakeCountStr;

      fopen_s(&quakeData, quakeDataFile.c_str(), "at");
      dataCount = data.size();
      for (size_t i = 0; i < dataCount; i++)
        fputs(data[i].c_str(), quakeData);

      data.clear();
      quakeCount++;
      fclose(quakeData);
    }

    data.push_back(buffer);
    lastTimeToFailure = stod(buffer.substr(sepPos + 1));
    count++;

    if ((count % 130000) == 0)
    {
      // save data
      quakeCountStr = to_string(quakeCount);
      quakeDataFile = kQuakeDataFile;
      if (quakeCountStr.size() == 1)
        quakeDataFile.push_back(kZero);
      quakeDataFile += quakeCountStr;

      fopen_s(&quakeData, quakeDataFile.c_str(), "at");
      dataCount = data.size();
      for (size_t i = 0; i < dataCount; i++)
        fputs(data[i].c_str(), quakeData);

      data.clear();
      fclose(quakeData);
    }
  }

  // save remaining data rows
  quakeCountStr = to_string(quakeCount);
  quakeDataFile = kQuakeDataFile;
  if (quakeCountStr.size() == 1)
    quakeDataFile.push_back(kZero);
  quakeDataFile += quakeCountStr;

  fopen_s(&quakeData, quakeDataFile.c_str(), "at");
  dataCount = data.size();
  for (size_t i = 0; i < dataCount; i++)
    fputs(data[i].c_str(), quakeData);

  fclose(quakeData);

  fclose(trainingData);
}

void reorderBasedOnLength()
{
  // reorder the data files based on length
  struct FileInfo
  {
    string filename;
    size_t size;
  };

  struct stat statBuffer;
  vector<FileInfo> fileSizes;
  FileInfo info;
  info.filename = kQuakeDataFile;
  info.filename += "00";

  stat(info.filename.c_str(), &statBuffer);
  info.size = statBuffer.st_size;
  fileSizes.push_back(info);
  for (size_t i = 1; i < kQuakeCount; i++)
  {
    info.filename = kQuakeDataFile;
    if (i < 10)
      info.filename.push_back('0');
    info.filename += to_string(i);

    stat(info.filename.c_str(), &statBuffer);
    info.size = statBuffer.st_size;

    for (size_t sIdx = 0; sIdx < fileSizes.size(); sIdx++)
      if (info.size > fileSizes[sIdx].size)
      {
        fileSizes.insert(fileSizes.begin() + sIdx, info);
        break;
      }
  }

  string quakeFile;
  for (size_t i = 0; i < kQuakeCount; i++)
  {
    quakeFile = kQuakeDataFileOrdered;
    if (i < 10)
      quakeFile.push_back('0');
    quakeFile += to_string(i);

    ifstream source(fileSizes[i].filename, ios::in | ios::binary);
    ofstream destination(quakeFile, ios::out | ios::binary);

    destination << source.rdbuf();
  }
}

void getValueFrequencies()
{
  // count the number each value occurs
  string buffer;
  buffer.resize(kBufferSize);
  FILE *trainingData = __nullptr;
  fopen_s(&trainingData, kTrainingDataFile, "rt");
  fgets(&buffer[0], kBufferSize, trainingData); // skip header

  unordered_map<int, size_t> valueFrequency;
  size_t sepPos = 0;
  while (fgets(&buffer[0], kBufferSize, trainingData))
  {
    sepPos = buffer.find(kSeparator);
    if (sepPos == string::npos)
      continue;

    valueFrequency[stoi(buffer.substr(0, sepPos))]++;
  }

  ofstream freqFile;
  freqFile.open(kFrequenciesFile);
  auto end = valueFrequency.end();
  for (auto it = valueFrequency.begin(); it != end; it++)
    freqFile << it->first << "," << it->second << endl;
  freqFile.close();

  trainingData = __nullptr;
  fopen_s(&trainingData, kFrequenciesFile, "rt");

  // sort the values by their occurence rate
  struct ValueFreq
  {
    int value;
    size_t frequency;

    ValueFreq(int iValue, size_t iFrequency)
        : value(iValue), frequency(iFrequency)
    {
    }
  };

  fgets(&buffer[0], kBufferSize, trainingData);
  vector<ValueFreq> sortedFreq;
  sortedFreq.push_back(ValueFreq(0, 0));
  while (fgets(&buffer[0], kBufferSize, trainingData))
  {
    sepPos = buffer.find(kSeparator);
    if (sepPos == string::npos)
      continue;

    int freq = stoi(buffer.substr(sepPos + 1, strlen(buffer.c_str())));
    for (size_t i = 0; i < sortedFreq.size(); i++)
      if (freq >= sortedFreq[i].frequency)
      {
        sortedFreq.insert(sortedFreq.begin() + i, ValueFreq(stoi(buffer.substr(0, sepPos)), stoi(buffer.substr(sepPos + 1, strlen(buffer.c_str())))));
        break;
      }
  }

  ofstream sortedFreqFile;
  sortedFreqFile.open(kSortedFreqFile);
  size_t count = sortedFreq.size();
  for (size_t i = 0; i < count; i++)
    sortedFreqFile << sortedFreq[i].value << " -> " << sortedFreq[i].frequency << endl;

  sortedFreqFile.flush();
  sortedFreqFile.close();

  fclose(trainingData);
}

void cleanDataFile(const string &iSourceDataFile, const string &iCleanDataFile)
{
  string buffer;
  buffer.resize(kBufferSize);
  FILE *sourceData = __nullptr;
  fopen_s(&sourceData, iSourceDataFile.c_str(), "rt");
  FILE *cleanData = __nullptr;
  fopen_s(&cleanData, iCleanDataFile.c_str(), "wt");

  int value = 0;
  size_t sepPos = 0;
  while (fgets(&buffer[0], kBufferSize, sourceData))
  {
    sepPos = buffer.find(kSeparator);
    if (sepPos == string::npos)
      continue;

    // clean noise
    value = stoi(buffer.substr(0, sepPos));
    if (value < 8)
      continue;

    // data seems symmetric, so we can remove the negative values
    if (value < 0)
      continue;

    fputs(buffer.c_str(), cleanData);
  }

  fclose(sourceData);
  fclose(cleanData);
}

void cleanData()
{
  string dataFile;
  string cleanFile;
  for (size_t i = 0; i < kQuakeCount; i++)
  {
    dataFile = kQuakeDataFileOrdered;
    cleanFile = kQuakeDataFileOrdClean;
    if (i < 10)
    {
      dataFile.push_back('0');
      cleanFile.push_back('0');
    }
    dataFile += to_string(i);
    cleanFile += to_string(i);

    cleanDataFile(dataFile, cleanFile);
  }
}

//----------------------------------------------------------------------

int main()
{
  // initial training preparation
  splitTrainingData();
  reorderBasedOnLength();
  getValueFrequencies();
  cleanData();

  return 0;
}
