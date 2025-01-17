#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "csshared/SharedTraits.h"
#include "csshared/DatabaseObserver.h"
#include "ConnectionObserver.h"

class LicenseManager;

//------------------------------------------------------------------------------
/**
 * A class which creates a single TCP connection for sending updates to a client.
 */
class UpdatesTCPConnection : public boost::enable_shared_from_this<UpdatesTCPConnection>,
                             public IDatabaseObserver,
                             public ConnectionObserverNotifier
{
public:
  typedef boost::shared_ptr<UpdatesTCPConnection> UpdatesTcpSharedPtr;

  /**
   * Create a new connection.
   */
  static UpdatesTcpSharedPtr Create(boost::asio::io_service &aIoService);

  /**
   * Get the current connection socket.
   */
  boost::asio::ip::tcp::socket &GetSocket();

  /**
   * Set the license manager.
   */
  void SetLicenseManager(LicenseManager *aLicenseManager);

  /**
   * Close the current connection.
   */
  void CloseConnection();

  //----------------------------------------------------------------------------
  // IDatabaseObserver implementation

  void RowUpdated(const std::wstring &aInformation);

  // End IDatabaseObserver implementation
  //----------------------------------------------------------------------------

private:
  static const int kTimeoutInSeconds;
  static const size_t kMessageBufferSize;

  boost::asio::io_service &mIoService;
  boost::asio::deadline_timer mDeadline;
  boost::system::error_code mConnectionError;

  boost::asio::ip::tcp::socket mMainSocket;
  LicenseManager *mLicenseManager;
  std::wstring mUpdateMsg;

  // class constructor
  UpdatesTCPConnection(boost::asio::io_service &aIoService);

  void SendDbUpdate(const std::wstring &aInformation);
  void AfterConnectionAttempt(const boost::system::error_code &aError,
                              boost::system::error_code *aOutError);
  void CheckDeadline();
  void AfterSendingUpdate(const boost::system::error_code &aError,
                          boost::system::error_code *aOutError);
};
