/* Copyright 2007 Andrew Diakin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ru.rpunet.webmops.utils;

import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * stores all application configuration
 * 
 * @author Andrew Diakin
 *
 */
public class ApplicationConfiguration {

	private static Log log = LogFactory.getLog(ApplicationConfiguration.class);
	
	private String configFileName = "/WEB-INF/app-config.xml";
	
	private boolean premoderation;
	
	private boolean registrationAllowed;
	
	private boolean trustedAllow;
	
	private List<String> anonymousUrls;
	
	private List<String> usersUrls;
	
	private List<String> moderatorsUrls;
	
	private String tempDir;
	
	private int avatarWidth;
	
	private int avatarHeight;

	public List<String> getAnonymousUrls() {
		return anonymousUrls;
	}

	public void setAnonymousUrls(List<String> anonymousUrls) {
		this.anonymousUrls = anonymousUrls;
	}

	public int getAvatarHeight() {
		return avatarHeight;
	}

	public void setAvatarHeight(int avatarHeight) {
		this.avatarHeight = avatarHeight;
	}

	public int getAvatarWidth() {
		return avatarWidth;
	}

	public void setAvatarWidth(int avatarWidth) {
		this.avatarWidth = avatarWidth;
	}

	public String getConfigFileName() {
		return configFileName;
	}

	public void setConfigFileName(String configFileName) {
		this.configFileName = configFileName;
	}

	public List<String> getModeratorsUrls() {
		return moderatorsUrls;
	}

	public void setModeratorsUrls(List<String> moderatorsUrls) {
		this.moderatorsUrls = moderatorsUrls;
	}

	public boolean isPremoderation() {
		return premoderation;
	}

	public void setPremoderation(boolean premoderation) {
		this.premoderation = premoderation;
	}

	public boolean isRegistrationAllowed() {
		return registrationAllowed;
	}

	public void setRegistrationAllowed(boolean registrationAllowed) {
		this.registrationAllowed = registrationAllowed;
	}

	public String getTempDir() {
		return tempDir;
	}

	public void setTempDir(String tempDir) {
		this.tempDir = tempDir;
	}

	public boolean isTrustedAllow() {
		return trustedAllow;
	}

	public void setTrustedAllow(boolean trustedAllow) {
		this.trustedAllow = trustedAllow;
	}

	public List<String> getUsersUrls() {
		return usersUrls;
	}

	public void setUsersUrls(List<String> usersUrls) {
		this.usersUrls = usersUrls;
	}
	
	public ApplicationConfiguration() {
		log.debug("Initializing application configurator.");
		//configFileName = "/opt/apache-tomcat-5.5.23/webapps/webmops" + configFileName; 
		configFileName = "/tmp/app-config.xml";
	}
	
	public void load(String configFileName) {
		
	}
	

	
	public void load() {
		load(configFileName);
	}
	
	public void save() {
		save(configFileName);
	}
	
	public void save(String configFileName) {
		
	}
}
