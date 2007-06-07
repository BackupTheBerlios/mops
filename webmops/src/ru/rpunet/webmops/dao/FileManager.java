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

package ru.rpunet.webmops.dao;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.hibernate.Session;

import ru.rpunet.webmops.model.File;
import ru.rpunet.webmops.utils.HibernateUtil;

/**
 * @author Andrew Diakin
 *
 */
public class FileManager {
	private Session session;
	
	public FileManager() {
		session = HibernateUtil.getSessionFactory().getCurrentSession();
	}
	
	public File findFileById(Long id) {
		return (File) session.load(File.class, id);
	}
	
	@SuppressWarnings("unchecked")
	public List<File> findAll() {
		List<File> files = (List<File>) session.createCriteria(File.class).list();
		return Collections.unmodifiableList(new ArrayList<File>(files));
	}
	
	public void saveOrUpdate(File file) {
		session.saveOrUpdate(file);
	}
	
	public void makePersistent(File file) {
		saveOrUpdate(file);
	}
}
