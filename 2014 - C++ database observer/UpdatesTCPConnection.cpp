#include <boost/bind.hpp>

#include "UpdatesTCPConnection.h"
#include "csshared/CommUtil.h"
#include "csshared/Version.h"
#include "server/LicenseManager.h"

using boost::asio::ip;

//------------------------------------------------------------------------------

const int UpdatesTCPConnection::kTimeoutInSeconds = 5;
const size_t UpdatesTCPConnection::kMessageBufferSize = 10000

    //------------------------------------------------------------------------------
    // Class implementation. Public methods.

    UpdatesTCPConnection::UpdatesTcpSharedPtr
    UpdatesTCPConnection::Create(boost::asio::io_service & aIoService)
{
  return UpdatesTcpSharedPtr(new UpdatesTCPConnection(aIoService));
}

tcp::socket &UpdatesTCPConnection::GetSocket()
{
  return mMainSocket;
}

void UpdatesTCPConnection::SetLicenseManager(LicenseManager *aLicenseManager)
{
  mLicenseManager = aLicenseManager;

  // send the license information and server version to the client
  mUpdateMsg = UTF16Vector::FromUTF8String(mLicenseManager->GetClientLicInfo());
  mUpdateMsg.push_back(static_cast<unsigned short>(kMsgTokenSep));
  UTF16Vector::Concatenate(mUpdateMsg, std::string(VersionInfo::kVersion));
  CommUtil::FormatStringMessage(mUpdateMsg);

  boost::system::error_code error;
  boost::asio::write(mMainSocket, boost::asio::buffer(&mUpdateMsg[0], ABUFSIZE_16(mUpdateMsg)), error);
}

void UpdatesTCPConnection::CloseConnection()
{
  // stop timeout handling and close socket
  mDeadline.cancel();
  boost::system::error_code error;
  mMainSocket.shutdown(tcp::socket::shutdown_receive, error);
  mMainSocket.close(error);
}

void UpdatesTCPConnection::RowUpdated(const std::wstring &aInformation)
{
  // send the update across the network
  SendDbUpdate(aInformation);
}

//------------------------------------------------------------------------------
// Class implementation. Private methods.

UpdatesTCPConnection::UpdatesTCPConnection(boost::asio::io_service &aIoService)
    : mIoService(aIoService), mDeadline(aIoService), mMainSocket(aIoService),
      mLicenseManager(NULL)
{
  // initialize main buffer and timeout handling
  mUpdateMsg.reserve(kMessageBufferSize);
  mDeadline.expires_at(boost::posix_time::pos_infin);
  CheckDeadline();
}

void UpdatesTCPConnection::SendDbUpdate(const std::wstring &aInformation)
{
  // format message
  mUpdateMsg = aInformation;
  CommUtil::FormatStringMessage(mUpdateMsg);

  // send the message asynchronously (other transfers may be in progress)
  mDeadline.expires_from_now(boost::posix_time::seconds(kTimeoutInSeconds));
  mConnectionError = boost::asio::error::would_block;

  boost::asio::async_write(mMainSocket,
                           boost::asio::buffer(&mUpdateMsg[0], ABUFSIZE_16(mUpdateMsg)),
                           boost::bind(&UpdatesTCPConnection::AfterSendingUpdate, shared_from_this(),
                                       boost::asio::placeholders::error, &mConnectionError));

  // wait for any transfer in progress and send the message
  do
    mIoService.run_one();
  while (mConnectionError == boost::asio::error::would_block);

  if (mConnectionError || !mMainSocket.is_open())
  {
    // client socket was lost, shut it down
    boost::system::error_code error;
    mMainSocket.shutdown(tcp::socket::shutdown_receive, error);
    mMainSocket.close(error);
    NotifyObserversClientNotResponding(reinterpret_cast<DWORD_PTR>(this));
  }
}

void UpdatesTCPConnection::AfterSendingUpdate(const boost::system::error_code &aError,
                                              boost::system::error_code *aOutError)
{
  // save error
  *aOutError = aError;
}

void UpdatesTCPConnection::CheckDeadline()
{
  if (!mMainSocket.is_open())
    return;

  // check if timeout was exceeded
  if (mDeadline.expires_at() <= boost::asio::deadline_timer::traits_type::now())
    mDeadline.expires_at(boost::posix_time::pos_infin);

  mDeadline.async_wait(boost::bind(&UpdatesTCPConnection::CheckDeadline, this));
}
