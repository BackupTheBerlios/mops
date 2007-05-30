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

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

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
		log.debug("Loading app-config.xml");
		DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
		Document document = null;
		
		try {
			DocumentBuilder builder = docFactory.newDocumentBuilder();
			document = builder.parse(new File(configFileName));
			
			Element root = (Element) document.getFirstChild();
			
			NodeList nodeList = root.getChildNodes();
			for ( int i = 0; i < nodeList.getLength(); i++) {
				Node node = nodeList.item(i);
				processNode(node);
			}
			
		} catch (Exception e) {
			e.printStackTrace();
		}
		
	}
	
	private void processNode(Node node) {
		log.debug("Processing " + node.getNodeName());
		if (node.getNodeName().equalsIgnoreCase("security")) {
			loadSecuritySettings(node);
		}
		
		if ( node.getNodeName().equalsIgnoreCase("settings") ) {
			loadSetting(node);
		}
	}
	
	private void loadSetting(Node node) {
		
		
	}
	
	private void loadSecuritySettings(Node node) {
		log.debug("Loading security settings");
		Node an = node.getFirstChild();
		String _url = null;
		int i = 0;
		
		if (an.getNodeName().equalsIgnoreCase("anonymous")) {
			// reading anonymous settings
			NodeList anList = an.getChildNodes();
			for (i = 0; i < anList.getLength(); i++) {
				if ( anonymousUrls == null )
					anonymousUrls = new ArrayList<String>();
				
				_url = anList.item(i).getNodeValue();
				log.debug("Adding " + _url + " to anonymous urls list.");
				anonymousUrls.add(anList.item(i).getNodeValue());
			}
		}
		
		if ( an.getNodeName().equalsIgnoreCase("user") ) {
			// reading user settings
			NodeList uList = an.getChildNodes();
			for (i = 0; i < uList.getLength(); i++) {
				if ( usersUrls == null ) {
					if (anonymousUrls != null) {
						usersUrls = anonymousUrls;
					} else {
						usersUrls = new ArrayList<String>();
					}
				}
				
				_url = uList.item(i).getNodeValue();
				log.debug("Adding " + _url + " to users urls list.");
				anonymousUrls.add(uList.item(i).getNodeValue());
					
			}
		}
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
