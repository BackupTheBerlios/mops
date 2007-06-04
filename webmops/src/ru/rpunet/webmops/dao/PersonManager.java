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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.Session;
import org.hibernate.criterion.Restrictions;

import ru.rpunet.webmops.model.Person;
import ru.rpunet.webmops.utils.HibernateUtil;
import ru.rpunet.webmops.utils.PasswordServiceImpl;
import ru.rpunet.webmops.utils.SystemUnavaliableException;

/**
 * @author Andrew Diakin
 *
 */
public class PersonManager {

	private static Log log = LogFactory.getLog(PersonManager.class);
	
	private Session session;
	
	public PersonManager() {
		session = HibernateUtil.getSessionFactory().getCurrentSession();
	}
	
	public Person findPerson(String name) {
		Person user = (Person) session.createCriteria(Person.class)
				.add(Restrictions.like("login", name)).uniqueResult();
		
		log.debug("For name " + name + " found user " + user);
		
		return user;
	}
	
	public Person findPersonById(Long id) {
		return (Person) session.load(Person.class, id);
	}
	
	@SuppressWarnings("unchecked")
	public List<Person> findAllPersons() {
		List<Person> users = (List<Person>) session.createCriteria(Person.class).list();
		return Collections.unmodifiableList(new ArrayList<Person>(users));
	}
	
	public void saveOrUpdate(Person user) throws SystemUnavaliableException {
		makePersistent(user);
	}
	
	public void makePersistent(Person user) throws SystemUnavaliableException {
		user.setPassword(PasswordServiceImpl.getInstance().encrypt(user.getPassword()));
		session.saveOrUpdate(user);
	}
}
