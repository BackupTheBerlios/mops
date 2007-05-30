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

/**
 * static configuration 
 * 
 * @author Andrew Diakin
 *
 */
public class AppConfigurator {

	private static ApplicationConfiguration config = new ApplicationConfiguration();
	
	static {
		config.load();
	}
	
	public static ApplicationConfiguration getConfig() {
		return config;
	}
	
	public static void reloadConfig() {
		config.load();
	}
	
}
