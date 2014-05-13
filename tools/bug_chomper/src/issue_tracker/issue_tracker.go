// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
	Utilities for interacting with the GoogleCode issue tracker.

	Example usage:
		issueTracker := issue_tracker.MakeIssueTraker(myOAuthConfigFile)
		authURL := issueTracker.MakeAuthRequestURL()
		// Visit the authURL to obtain an authorization code.
		issueTracker.UpgradeCode(code)
		// Now issueTracker can be used to retrieve and edit issues.
*/
package issue_tracker

import (
	"bytes"
	"code.google.com/p/goauth2/oauth"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
	"strconv"
	"strings"
)

// BugPriorities are the possible values for "Priority-*" labels for issues.
var BugPriorities = []string{"Critical", "High", "Medium", "Low", "Never"}

var apiScope = []string{
	"https://www.googleapis.com/auth/projecthosting",
	"https://www.googleapis.com/auth/userinfo.email",
}

const issueApiURL = "https://www.googleapis.com/projecthosting/v2/projects/"
const issueURL = "https://code.google.com/p/skia/issues/detail?id="
const personApiURL = "https://www.googleapis.com/userinfo/v2/me"

// Enum for determining whether a label has been added, removed, or is
// unchanged.
const (
	labelAdded = iota
	labelRemoved
	labelUnchanged
)

// loadOAuthConfig reads the OAuth given config file path and returns an
// appropriate oauth.Config.
func loadOAuthConfig(oauthConfigFile string) (*oauth.Config, error) {
	errFmt := "failed to read OAuth config file: %s"
	fileContents, err := ioutil.ReadFile(oauthConfigFile)
	if err != nil {
		return nil, fmt.Errorf(errFmt, err)
	}
	var decodedJson map[string]struct {
		AuthURL      string `json:"auth_uri"`
		ClientId     string `json:"client_id"`
		ClientSecret string `json:"client_secret"`
		TokenURL     string `json:"token_uri"`
	}
	if err := json.Unmarshal(fileContents, &decodedJson); err != nil {
		return nil, fmt.Errorf(errFmt, err)
	}
	config, ok := decodedJson["web"]
	if !ok {
		return nil, fmt.Errorf(errFmt, err)
	}
	return &oauth.Config{
		ClientId:     config.ClientId,
		ClientSecret: config.ClientSecret,
		Scope:        strings.Join(apiScope, " "),
		AuthURL:      config.AuthURL,
		TokenURL:     config.TokenURL,
	}, nil
}

// Issue contains information about an issue.
type Issue struct {
	Id      int      `json:"id"`
	Project string   `json:"projectId"`
	Title   string   `json:"title"`
	Labels  []string `json:"labels"`
}

// URL returns the URL of a given issue.
func (i Issue) URL() string {
	return issueURL + strconv.Itoa(i.Id)
}

// IssueList represents a list of issues from the IssueTracker.
type IssueList struct {
	TotalResults int      `json:"totalResults"`
	Items        []*Issue `json:"items"`
}

// IssueTracker is the primary point of contact with the issue tracker,
// providing methods for authenticating to and interacting with it.
type IssueTracker struct {
	OAuthConfig    *oauth.Config
	OAuthTransport *oauth.Transport
}

// MakeIssueTracker creates and returns an IssueTracker with authentication
// configuration from the given authConfigFile.
func MakeIssueTracker(authConfigFile string, redirectURL string) (*IssueTracker, error) {
	oauthConfig, err := loadOAuthConfig(authConfigFile)
	if err != nil {
		return nil, fmt.Errorf(
			"failed to create IssueTracker: %s", err)
	}
	oauthConfig.RedirectURL = redirectURL
	return &IssueTracker{
		OAuthConfig:    oauthConfig,
		OAuthTransport: &oauth.Transport{Config: oauthConfig},
	}, nil
}

// MakeAuthRequestURL returns an authentication request URL which can be used
// to obtain an authorization code via user sign-in.
func (it IssueTracker) MakeAuthRequestURL() string {
	// NOTE: Need to add XSRF protection if we ever want to run this on a public
	// server.
	return it.OAuthConfig.AuthCodeURL(it.OAuthConfig.RedirectURL)
}

// IsAuthenticated determines whether the IssueTracker has sufficient
// permissions to retrieve and edit Issues.
func (it IssueTracker) IsAuthenticated() bool {
	return it.OAuthTransport.Token != nil
}

// UpgradeCode exchanges the single-use authorization code, obtained by
// following the URL obtained from IssueTracker.MakeAuthRequestURL, for a
// multi-use, session token. This is required before IssueTracker can retrieve
// and edit issues.
func (it *IssueTracker) UpgradeCode(code string) error {
	token, err := it.OAuthTransport.Exchange(code)
	if err == nil {
		it.OAuthTransport.Token = token
		return nil
	} else {
		return fmt.Errorf(
			"failed to exchange single-user auth code: %s", err)
	}
}

// GetLoggedInUser retrieves the email address of the authenticated user.
func (it IssueTracker) GetLoggedInUser() (string, error) {
	errFmt := "error retrieving user email: %s"
	if !it.IsAuthenticated() {
		return "", fmt.Errorf(errFmt, "User is not authenticated!")
	}
	resp, err := it.OAuthTransport.Client().Get(personApiURL)
	if err != nil {
		return "", fmt.Errorf(errFmt, err)
	}
	defer resp.Body.Close()
	body, _ := ioutil.ReadAll(resp.Body)
	if resp.StatusCode != http.StatusOK {
		return "", fmt.Errorf(errFmt, fmt.Sprintf(
			"user data API returned code %d: %v", resp.StatusCode, string(body)))
	}
	userInfo := struct {
		Email string `json:"email"`
	}{}
	if err := json.Unmarshal(body, &userInfo); err != nil {
		return "", fmt.Errorf(errFmt, err)
	}
	return userInfo.Email, nil
}

// GetBug retrieves the Issue with the given ID from the IssueTracker.
func (it IssueTracker) GetBug(project string, id int) (*Issue, error) {
	errFmt := fmt.Sprintf("error retrieving issue %d: %s", id, "%s")
	if !it.IsAuthenticated() {
		return nil, fmt.Errorf(errFmt, "user is not authenticated!")
	}
	requestURL := issueApiURL + project + "/issues/" + strconv.Itoa(id)
	resp, err := it.OAuthTransport.Client().Get(requestURL)
	if err != nil {
		return nil, fmt.Errorf(errFmt, err)
	}
	defer resp.Body.Close()
	body, _ := ioutil.ReadAll(resp.Body)
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf(errFmt, fmt.Sprintf(
			"issue tracker returned code %d:%v", resp.StatusCode, string(body)))
	}
	var issue Issue
	if err := json.Unmarshal(body, &issue); err != nil {
		return nil, fmt.Errorf(errFmt, err)
	}
	return &issue, nil
}

// GetBugs retrieves all Issues with the given owner from the IssueTracker,
// returning an IssueList.
func (it IssueTracker) GetBugs(project string, owner string) (*IssueList, error) {
	errFmt := "error retrieving issues: %s"
	if !it.IsAuthenticated() {
		return nil, fmt.Errorf(errFmt, "user is not authenticated!")
	}
	params := map[string]string{
		"owner":      url.QueryEscape(owner),
		"can":        "open",
		"maxResults": "9999",
	}
	requestURL := issueApiURL + project + "/issues?"
	first := true
	for k, v := range params {
		if first {
			first = false
		} else {
			requestURL += "&"
		}
		requestURL += k + "=" + v
	}
	resp, err := it.OAuthTransport.Client().Get(requestURL)
	if err != nil {
		return nil, fmt.Errorf(errFmt, err)
	}
	defer resp.Body.Close()
	body, _ := ioutil.ReadAll(resp.Body)
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf(errFmt, fmt.Sprintf(
			"issue tracker returned code %d:%v", resp.StatusCode, string(body)))
	}

	var bugList IssueList
	if err := json.Unmarshal(body, &bugList); err != nil {
		return nil, fmt.Errorf(errFmt, err)
	}
	return &bugList, nil
}

// SubmitIssueChanges creates a comment on the given Issue which modifies it
// according to the contents of the passed-in Issue struct.
func (it IssueTracker) SubmitIssueChanges(issue *Issue, comment string) error {
	errFmt := "Error updating issue " + strconv.Itoa(issue.Id) + ": %s"
	if !it.IsAuthenticated() {
		return fmt.Errorf(errFmt, "user is not authenticated!")
	}
	oldIssue, err := it.GetBug(issue.Project, issue.Id)
	if err != nil {
		return fmt.Errorf(errFmt, err)
	}
	postData := struct {
		Content string `json:"content"`
		Updates struct {
			Title  *string  `json:"summary"`
			Labels []string `json:"labels"`
		} `json:"updates"`
	}{
		Content: comment,
	}
	if issue.Title != oldIssue.Title {
		postData.Updates.Title = &issue.Title
	}
	// TODO(borenet): Add other issue attributes, eg. Owner.
	labels := make(map[string]int)
	for _, label := range issue.Labels {
		labels[label] = labelAdded
	}
	for _, label := range oldIssue.Labels {
		if _, ok := labels[label]; ok {
			labels[label] = labelUnchanged
		} else {
			labels[label] = labelRemoved
		}
	}
	labelChanges := make([]string, 0)
	for labelName, present := range labels {
		if present == labelRemoved {
			labelChanges = append(labelChanges, "-"+labelName)
		} else if present == labelAdded {
			labelChanges = append(labelChanges, labelName)
		}
	}
	if len(labelChanges) > 0 {
		postData.Updates.Labels = labelChanges
	}

	postBytes, err := json.Marshal(&postData)
	if err != nil {
		return fmt.Errorf(errFmt, err)
	}
	requestURL := issueApiURL + issue.Project + "/issues/" +
		strconv.Itoa(issue.Id) + "/comments"
	resp, err := it.OAuthTransport.Client().Post(
		requestURL, "application/json", bytes.NewReader(postBytes))
	if err != nil {
		return fmt.Errorf(errFmt, err)
	}
	defer resp.Body.Close()
	body, _ := ioutil.ReadAll(resp.Body)
	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf(errFmt, fmt.Sprintf(
			"Issue tracker returned code %d:%v", resp.StatusCode, string(body)))
	}
	return nil
}
