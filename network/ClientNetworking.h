#ifndef _ClientNetworking_h_
#define _ClientNetworking_h_

#include <boost/optional/optional_fwd.hpp>

#include <string>
#include <vector>
#include <memory>
#include <chrono>

#include <memory>

class Message;

/** Encapsulates the networking facilities of the client.  The client must
    execute its networking code in a separate thread from its main processing
    thread, for UI and networking responsiveness, and to process synchronous
    communication with the server.

    Because of this, ClientNetworking operates in two threads: the main
    thread, in which the UI processing operates; the networking thread, which
    is created and terminates when the client is connected to and disconnected
    from the server, respectively.  The entire public interface is safe to
    call from the main thread at all times.  Note that the main thread must
    periodically request the next incoming message; the arrival of incoming
    messages is never explicitly signalled to the main thread.  The same
    applies to unintentional disconnects from the server.  The client must
    periodically check IsConnected().

    The ClientNetworking has three modes of operation.  First, it can discover
    FreeOrion servers on the local network; this is a blocking operation with
    a timeout.  Second, it can send an asynchronous message to the server
    (when connected); this is a non-blocking operation.  Third, it can send a
    synchronous message to the server (when connected) and return the server's
    response; this is a blocking operation.

    Note that the SendSynchronousMessage() does not interrupt the sending or
    receiving of asynchronous messages.  When SendSynchronousMessage() is
    called and the main thread blocks to wait for the response message,
    regular messages are still being sent and received.  Some of these regular
    messages may arrive before the response message, but
    SendSynchronousMessage() will still return the response message first,
    even if it is not at the front of the queue at the time.  This implies
    that some response messages may be handled out of order with respect to
    regular messages, but these are in fact the desired semantics. */
class ClientNetworking : public std::enable_shared_from_this<ClientNetworking> {
public:
    /** The type of list returned by a call to DiscoverLANServers(). */
    using ServerNames =  std::vector<std::string>;

    /** \name Structors */ //@{
    ClientNetworking();
    ~ClientNetworking();
    //@}

    /** \name Accessors */ //@{
    /** Returns true iff the client is full duplex connected to the server. */
    bool IsConnected() const;

    /** Returns true iff the client is connected to receive from the server. */
    bool IsRxConnected() const;

    /** Returns true iff the client is connected to send to the server. */
    bool IsTxConnected() const;

    /** Returns the ID of the player on this client. */
    int PlayerID() const;

    /** Returns the ID of the host player, or INVALID_PLAYER_ID if there is no host player. */
    int HostPlayerID() const;

    /** Returns whether the indicated player ID is the host. */
    bool PlayerIsHost(int player_id) const;
    //@}

    /** \name Mutators */ //@{
    /** Returns a list of the addresses and names of all servers on the Local
        Area Network. */
    ServerNames DiscoverLANServerNames();

    /** Connects to the server at \a ip_address.  On failure, repeated
        attempts will be made until \a timeout seconds has elapsed. */
    bool ConnectToServer(const std::string& ip_address,
                         const std::chrono::milliseconds& timeout = std::chrono::seconds(10));

    /** Connects to the server on the client's host.  On failure, repeated
        attempts will be made until \a timeout seconds has elapsed. */
    bool ConnectToLocalHostServer(const std::chrono::milliseconds& timeout =
                                  std::chrono::seconds(10));

    /** Sends \a message to the server.  This function actually just enqueues
        the message for sending and returns immediately. */
    void SendMessage(const Message& message);

    /** Return the next incoming message from the server if available or boost::none.
        Remove the message from the incoming message queue. */
    boost::optional<Message> GetMessage();

    /** Sends \a message to the server, then blocks until it sees the first
        synchronous response from the server. */
    boost::optional<Message> SendSynchronousMessage(const Message& message);

    /** Disconnects the client from the server. First tries to send any pending transmit messages. */
    void DisconnectFromServer();

    /** Sets player ID for this client. */
    void SetPlayerID(int player_id);

    /** Sets Host player ID. */
    void SetHostPlayerID(int host_player_id);
    //@}

private:
    class Impl;
    std::unique_ptr<Impl> const m_impl;
};

#endif
