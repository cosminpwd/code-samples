#pragma once

#include <string>
#include <vector>

//------------------------------------------------------------------------------
/**
 * An interface for a database observer.
 */
class IDatabaseObserver
{
public:
  /**
   * The database was updated.
   */
  virtual void RowUpdated(const std::wstring &aInformation) = 0;
};

//------------------------------------------------------------------------------
/**
 * A class which manages database observers.
 */
class DatabaseObserverNotifier
{
public:
  /**
   * Add an observer to the notification list.
   */
  void AddObserver(IDatabaseObserver *aObserver)
  {
    mObservers.push_back(aObserver);
  }

  /**
   * Remove an observer from the notification list.
   */
  void RemoveObserver(IDatabaseObserver *aObserver)
  {
    size_t count = mObservers.size();
    for (size_t i = 0; i < count; i++)
      if (mObservers[i] == aObserver)
      {
        mObservers.erase(mObservers.begin() + i);
        break;
      }
  }

protected:
  /**
   * Notify observers that an existing row was updated.
   * One of the observers is responsible for updating the row, so we
   * should skip it because it already knows.
   */
  void NotifyObserversUpdate(IDatabaseObserver *aSourceObserver)
  {
    size_t count = mObservers.size();
    for (size_t i = 0; i < count; i++)
      if (mObservers[i] != aSourceObserver)
        mObservers[i]->RowUpdated(mObservedInfo);
  }

  /**
   * Notify all observers that a new recorded action (history) was inserted
   * into the database.
   */
  void NotifyObserversNewHistory()
  {
    size_t count = mObservers.size();
    for (size_t i = 0; i < count; i++)
      mObservers[i]->RowUpdated(mObservedInfo);
  }

  std::wstring mObservedInfo;

private:
  std::vector<IDatabaseObserver *> mObservers;
};
