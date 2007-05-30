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
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.Session;
import org.hibernate.criterion.Restrictions;

import ru.rpunet.webmops.model.Package;
import ru.rpunet.webmops.utils.HibernateUtil;

/**
 * @author Andrew Diakin
 *
 */
public class PackageManager {
	private Map<Long, Package> packages;
	private Session session;
	
	private static Log log = LogFactory.getLog(PackageManager.class);
	
	@SuppressWarnings("unchecked")
	public PackageManager() {
		log.debug("Creating PackageManager");
		packages = new HashMap<Long, Package>();
		session = HibernateUtil.getSessionFactory().getCurrentSession();
		List<Package> pkgs = (List<Package>) session.createCriteria(Package.class).list();
		for (Package pkg : pkgs) {
			saveOrUpdate(pkg);
		}
	}
	
	public Package findById(Long id) {
		return packages.get(id);
	}
	
	public List<Package> findAll() {
		return Collections.unmodifiableList(new ArrayList<Package>(packages.values()));
	}
	
	public void saveOrUpdate(Package pkg) {
		packages.put(pkg.getId(), pkg);
	}
	
	public void makePersistent(Package pkg) {
		session.save(pkg);
		saveOrUpdate(pkg);
	}
	
	@SuppressWarnings("unchecked")
	public List<Package> findPackagesByName(String name) {
		List<Package> pkgs = (List<Package>) session.createCriteria(Package.class)
					.add(Restrictions.like("name",name + "%")).list();
		
		return pkgs;
	}


	/**
	 * Search package by version and build
	 * @param version package version
	 * @param build package build
	 * @return package 
	 */
	@SuppressWarnings("unchecked")
	public List<Package> findPackageByVer(String version, String build) {
		List<Package> pkg = (List<Package>) session.createCriteria(Package.class)
					.add(Restrictions.like("version", version))
					.add(Restrictions.like("build", build))
					.list();
		return pkg;
	}
	
	
}
