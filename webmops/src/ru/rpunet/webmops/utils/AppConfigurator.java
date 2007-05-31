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

import java.util.ArrayList;
import java.util.List;

/**
 * static configuration 
 * 
 * @author Andrew Diakin
 *
 */
public class AppConfigurator {

	private static ApplicationConfiguration config = new ApplicationConfiguration();
	
	private static List<String> anonymousUrls = new ArrayList<String>();
	private static List<String> usersUrls = new ArrayList<String>();
	private static List<String> moderatorsUrls = new ArrayList<String>();
	
	static {
		anonymousUrls.add("/List.action");
		anonymousUrls.add("/users/Registed.action");
		anonymousUrls.add("/users/Login.action");
		anonymousUrls.add("/Search.action");
		anonymousUrls.add("/PackageInfo.action");
		anonymousUrls.add("/ShowFiles.action");
		anonymousUrls.add("/Default.action");
		anonymousUrls.add("/CssResourcesLoader.action");
		
		usersUrls = anonymousUrls;
		usersUrls.add("/Logout.action");
		usersUrls.add("/UploadPackage.action");
		usersUrls.add("/users/Profile.action");
		usersUrls.add("/users/EditProfile.action");
		usersUrls.add("/users/UploadPhoto.action");
		usersUrls.add("/users/ListPackages.action");
		usersUrls.add("/users/RemovePackage.action");
		
		moderatorsUrls = usersUrls;
		moderatorsUrls.add("/EditPackage.action");
		moderatorsUrls.add("/DeletePackage.action");
		moderatorsUrls.add("/RebuildIndex.action");
		moderatorsUrls.add("/ViewPackage.action");
		moderatorsUrls.add("/AllowPackage.action");
		moderatorsUrls.add("/users/List.action");
		moderatorsUrls.add("/users/BlockUser.action");
		moderatorsUrls.add("/users/EditUser.action");

	}
	
	static {
		config.load();
		config.setAnonymousUrls(anonymousUrls);
		config.setUsersUrls(usersUrls);
		config.setModeratorsUrls(moderatorsUrls);
		
	}
	
	public static ApplicationConfiguration getConfig() {
		return config;
	}
	
	public static void reloadConfig() {
		config.load();
	}
	
}
