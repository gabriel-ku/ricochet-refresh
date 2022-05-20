// Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "IdentityManager.h"
#include "ContactIDValidator.h"
#include "ContactUser.h"
#include "core/OutgoingContactRequest.h"

IdentityManager *identityManager = 0;

IdentityManager::IdentityManager(const QString& serviceID, QObject *parent)
    : QObject(parent), highestID(-1)
{
    identityManager = this;

    if (serviceID.isEmpty())
    {
        createIdentity();
    }
    else
    {
        addIdentity(new UserIdentity(0, serviceID, this));
    }
}

IdentityManager::~IdentityManager()
{
    identityManager = 0;
}

void IdentityManager::addIdentity(UserIdentity *identity)
{
    m_identities.append(identity);
    highestID = qMax(identity->uniqueID, highestID);

    connect(&identity->contacts, SIGNAL(outgoingRequestAdded(OutgoingContactRequest*)),
            SLOT(onOutgoingRequest(OutgoingContactRequest*)));
    connect(&identity->contacts.incomingRequests, SIGNAL(requestAdded(IncomingContactRequest*)),
            SLOT(onIncomingRequest(IncomingContactRequest*)));
    connect(&identity->contacts.incomingRequests, SIGNAL(requestRemoved(IncomingContactRequest*)),
            SLOT(onIncomingRequestRemoved(IncomingContactRequest*)));
}

UserIdentity *IdentityManager::createIdentity()
{
    UserIdentity *identity = UserIdentity::createIdentity(++highestID);
    if (!identity)
        return identity;

    addIdentity(identity);

    return identity;
}

UserIdentity *IdentityManager::lookupHostname(const QString &hostname) const
{
    QString ohost = ContactIDValidator::hostnameFromID(hostname);
    if (ohost.isNull())
        ohost = hostname;

    if (!ohost.endsWith(QLatin1String(".onion")))
        ohost.append(QLatin1String(".onion"));

    for (QList<UserIdentity*>::ConstIterator it = m_identities.begin(); it != m_identities.end(); ++it)
    {
        if (ohost.compare((*it)->hostname(), Qt::CaseInsensitive) == 0)
            return *it;
    }

    return 0;
}

UserIdentity *IdentityManager::lookupUniqueID(int uniqueID) const
{
    for (QList<UserIdentity*>::ConstIterator it = m_identities.begin(); it != m_identities.end(); ++it)
    {
        if ((*it)->uniqueID == uniqueID)
            return *it;
    }

    return 0;
}

void IdentityManager::onOutgoingRequest(OutgoingContactRequest *request)
{
    emit outgoingRequestAdded(request, request->user->identity);
}

void IdentityManager::onIncomingRequest(IncomingContactRequest *request)
{
    emit incomingRequestAdded(request, request->manager->contacts->identity);
}

void IdentityManager::onIncomingRequestRemoved(IncomingContactRequest *request)
{
    emit incomingRequestRemoved(request, request->manager->contacts->identity);
}
