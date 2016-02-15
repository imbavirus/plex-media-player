//
// Created by Tobias Hieta on 20/08/15.
//

#include "CachedRegexMatcher.h"
#include "QsLog.h"

/////////////////////////////////////////////////////////////////////////////////////////
bool CachedRegexMatcher::addMatcher(const QString& pattern, const QVariant& result)
{
  QRegExp matcher(pattern);
  if (!matcher.isValid())
  {
    QLOG_WARN() << "Could not compile pattern:" << pattern;
    return false;
  }

  m_matcherList.push_back(qMakePair(matcher, result));
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
QVariant CachedRegexMatcher::match(const QString& input)
{
  // first we check if this match has already happened before
  if (m_matcherCache.contains(input))
    return m_matcherCache.value(input);

  // otherwise try to iterate our list and find a match
  foreach(const MatcherValuePair& matcher, m_matcherList)
  {
    QRegExp re(matcher.first);

    if (re.indexIn(input) != -1)
    {
      // found match
      QVariant returnValue = matcher.second;

      if (re.captureCount() > 0 && matcher.second.type() == QVariant::String)
      {
        QString value(matcher.second.toString());

        for (int i = 0; i < re.captureCount(); i ++)
        {
          QString argFmt = QString("%%1").arg(i + 1);
          if (value.contains(argFmt))
            value = value.arg(re.cap(i + 1));
        }
        returnValue = QVariant(value);
      }

      // now cache the match and the final value
      m_matcherCache.insert(input, returnValue);

      return returnValue;
    }
  }

  QLOG_DEBUG() << "No match for:" << input;

  // no match at all
  return QVariant();
}

/////////////////////////////////////////////////////////////////////////////////////////
void CachedRegexMatcher::clear()
{
  m_matcherCache.clear();
  m_matcherList.clear();
}
