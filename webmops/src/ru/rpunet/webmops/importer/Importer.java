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

package ru.rpunet.webmops.importer;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.xml.XmlBeanFactory;
import org.springframework.core.io.ClassPathResource;
import org.springframework.core.io.Resource;

import ru.rpunet.webmops.dao.FileManager;
import ru.rpunet.webmops.dao.PackageManager;

/**
 * 
 * Read packages from repository and put it to db
 * 
 * @author Andrew Diakin
 *
 */
public class Importer {
	private PackageManager packageDAO;
	private FileManager fileDAO;

	private PackageListReader packagesListReader;
	
	public Importer() {
		packageDAO = new PackageManager();
		fileDAO = new FileManager();
		
		packagesListReader = new SlackwarePackagesListReaderImpl();
	}
	
	public void importPackages() {
		Resource resource = new ClassPathResource("/WEB-INF/spring-bean.xml");
		BeanFactory factory = new XmlBeanFactory(resource);
	}
}
