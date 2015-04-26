/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

/**
    Contains information about whether your app has been unlocked for the current machine,
    and handles communication with the web-store to perform the unlock procedure.

    To use this class, create a subclass of it, and implement its pure virtual methods
    (see their comments to find out what you'll need to make them do).

    Then you can create an instance of your subclass which will hold the registration
    state. Typically, you'll want to just keep a single instance of the class around for
    the duration of your app. You can then call its methods to handle the various
    registration tasks.

    Areas of your code that need to know whether the user is registered (e.g. to decide
    whether a particular feature is available) should call isUnlocked() to find out.

    If you want to create a GUI that allows your users to enter their details and
    register, see the TracktionMarketplaceUnlockForm class.

    @see TracktionMarketplaceUnlockForm, TracktionMarketplaceKeyGeneration
*/
class JUCE_API  TracktionMarketplaceStatus
{
public:
    TracktionMarketplaceStatus();

    /** Destructor. */
    virtual ~TracktionMarketplaceStatus();

    //==============================================================================
    /** This must return your product's ID, as allocated by the store. */
    virtual String getMarketplaceProductID() = 0;

    /** This checks whether a product ID string that the server returned is OK for
        unlocking the current app.
        By default this just compares the string with getMarketplaceProductID() but you
        may want to add more custom behaviour.
    */
    virtual bool doesMarketplaceProductIDMatch (const String& returnedIDFromServer);

    /** This must return the RSA public key for authenticating responses from
        the server for this app. You can get this key from your marketplace
        account page.
    */
    virtual RSAKey getPublicKey() = 0;

    /** This method must store the given string somewhere in your app's
        persistent properties, so it can be retrieved later by getState().
    */
    virtual void saveState (const String&) = 0;

    /** This method must retrieve the last state that was provided by the
        saveState method.

        On first-run, it should just return an empty string.
    */
    virtual String getState() = 0;

    /** Returns a list of strings, any of which should be unique to this
        physical computer.

        When testing whether the user is allowed to use the product on this
        machine, this list of tokens is compared to the ones that were stored
        on the Tracktion marketplace webserver.

        The default implementation of this method will calculate some
        machine IDs based on things like network MAC addresses, hard-disk
        IDs, etc, but if you want, you can overload it to generate your
        own list of IDs.

        The IDs that are returned should be short alphanumeric strings
        without any punctuation characters. Since users may need to type
        them, case is ignored when comparing them.

        Note that the first item in the list is considered to be the
        "main" ID, and this will be the one that is displayed to the user
        and registered with the marketplace webserver. Subsequent IDs are
        just used as fallback to avoid false negatives when checking for
        registration on machines which have had hardware added/removed
        since the product was first registered.
    */
    virtual StringArray getLocalMachineIDs();

    /** Can be overridden if necessary, but by default returns the tracktion.com
        marketplace server.
    */
    virtual URL getServerAuthenticationURL();

    /** Can be overridden if necessary, but by default returns "tracktion.com". */
    virtual String getWebsiteName();

    /** The default implementation of this method will construct a URL with the default
        parameters and read the reply, but for custom webserver set-ups, you may need to
        override it to use more exotic methods. */
    virtual String readReplyFromWebserver (const String& email, const String& password);

    //==============================================================================
    // The following methods can be called by your app:

    /** Returns true if the product has been successfully authorised for this machine.

        The reason it returns a variant rather than a bool is just to make it marginally
        more tedious for crackers to work around. Hopefully if this method gets inlined
        they'll need to hack all the places where you call it, rather than just the
        function itself.

        Bear in mind that each place where you check this return value will need to be
        changed by a cracker in order to unlock your app, so the more places you call this
        method, the more hassle it will be for them to find and crack them all.
    */
    inline var isUnlocked() const       { return status[unlockedProp]; }

    /** Optionally allows the app to provide the user's email address if
        it is known.
        You don't need to call this, but if you do it may save the user
        typing it in.
    */
    void setUserEmail (const String& usernameOrEmail);

    /** Returns the user's email address if known. */
    String getUserEmail() const;

    /** Attempts to perform an unlock using a block of key-file data provided.
        You may wish to use this as a way of allowing a user to unlock your app
        by drag-and-dropping a file containing the key data, or by letting them
        select such a file. This is often needed for allowing registration on
        machines without internet access.
    */
    bool applyKeyFile (String keyFileContent);

    /** This provides some details about the reply that the server gave in a call
        to attemptWebserverUnlock().
    */
    struct UnlockResult
    {
        /** If an unlock operation fails, this is the error message that the webserver
            supplied (or a message saying that the server couldn't be contacted)
        */
        String errorMessage;

        /** This is a message that the webserver returned, and which the user should
            be shown.

            It's not necessarily an error message, e.g. it might say that there's a
            new version of the app available or some other status update.
        */
        String informativeMessage;

        /** If the webserver wants the user to be directed to a web-page for further
            information, this is the URL that it would like them to go to.
        */
        String urlToLaunch;

        /** If the unlock operation succeeded, this will be set to true. */
        bool succeeded;
    };

    /** Contacts the webserver and attempts to perform a registration with the
        given user details.

        The return value will either be a success, or a failure with an error message
        from the server, so you should show this message to your user.

        Note that this is designed for the Tracktion marketplace server - if you're
        building your own server, this function probably won't do the right thing for you.
    */
    UnlockResult attemptWebserverUnlock (const String& email, const String& password);

    /** Attempts to load the status from the state retrieved by getState().
        Call this somewhere in your app's startup code.
     */
    void load();

    /** Triggers a call to saveState which you can use to store the current unlock status
        in your app's settings.
     */
    void save();

private:
    ValueTree status;

    UnlockResult handleXmlReply (XmlElement);
    UnlockResult handleFailedConnection();

    static const char* unlockedProp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TracktionMarketplaceStatus)
};
