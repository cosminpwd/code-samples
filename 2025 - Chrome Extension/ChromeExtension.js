// ...

const kListItemEl = "li";
const kSpanEl = "span";
const kLinkEl = "a";

const kSeparatorText = " · ";
const kEmptyHref = "#";
const kShowLabel = "Show";

const kCompaniesStorageKey = "companies";
const kCompaniesSHAStorageKey = "companiesSHA";

const kUpdateMessage = "updatePage";
const kRemoveMessage = "removeCompany";

const kTargetUrl = "example.com";

// ...

function populateCompaniesList(companyListEl) {
  let compCount = mFilteredCompanies.length;

  // show message if there are no companies matching the filter
  if (!compCount) {
    let noResultsEl = document.createElement(kListItemEl);
    noResultsEl.innerText = kNoResultsText;
    noResultsEl.classList = kListItemClass;
    companyListEl.prepend(noResultsEl);

    return;
  }

  for (let i = 0; i < compCount; i++) {
    let companyEl = document.createElement(kListItemEl);
    companyEl.innerText = mFilteredCompanies[i];
    companyEl.classList = kListItemClass;

    // add a show option for this company
    let showCompanyEl = document.createElement(kSpanEl);
    showCompanyEl.innerText = kSeparatorText;
    companyEl.append(showCompanyEl);

    let showCompanyLinkEl = document.createElement(kLinkEl);
    showCompanyLinkEl.href = kEmptyHref;
    showCompanyLinkEl.classList = kShowCompanyClass;
    showCompanyLinkEl.innerText = kShowLabel;
    showCompanyLinkEl.setAttribute(kCompanyNameAttr, mFilteredCompanies[i]);
    showCompanyEl.append(showCompanyLinkEl);

    companyListEl.prepend(companyEl);

    showCompanyLinkEl.addEventListener("click", (event) => {
      event.stopPropagation();
      let companyName = event.target.getAttribute(kCompanyNameAttr);
      mCompanies = mCompanies.filter(item => item !== companyName);

      chrome.storage.local.set({ kCompaniesStorageKey: mCompanies }).then(() => {
        if (!mToken) {
          // update page
          chrome.tabs.query({ active: true, currentWindow: true }, function (tabs) {
            if (tabs[0] && tabs[0].id && tabs[0].url.includes(kTargetUrl)) chrome.tabs.sendMessage(tabs[0].id, kUpdateMessage);
          });
        } else {
          // remove company from list
          getSHA256(JSON.stringify(mCompanies)).then(result => chrome.storage.local.set({ kCompaniesSHAStorageKey: result }));
          let delCompData = {
            "companies": JSON.stringify(companyName),
            "email": mEmail,
            "emailId": mEmailId,
          }
          chrome.runtime.sendMessage({ action: kRemoveMessage, data: { body: JSON.stringify(delCompData), token: mToken } });

          // update page
          chrome.tabs.query({ active: true, currentWindow: true }, function (tabs) {
            if (tabs[0] && tabs[0].id && tabs[0].url.includes(kTargetUrl)) chrome.tabs.sendMessage(tabs[0].id, kUpdateMessage);
          });
        }
      });
    })
  }
}

// ...