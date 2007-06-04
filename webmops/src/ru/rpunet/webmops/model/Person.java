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
                  
package ru.rpunet.webmops.model;


import java.io.Serializable;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Table;
import javax.persistence.Id;
import javax.persistence.UniqueConstraint;

import org.hibernate.validator.NotNull;



/**
 * Person entity
 * 
 * @author Andrew Diakin
 *
 */
@Entity
@Table(
		name="users",
		uniqueConstraints=
			@UniqueConstraint(columnNames={"login", "email"}))
public class Person implements Serializable {

	/**
	 * 
	 */
	private static final long serialVersionUID = 8754690924525353187L;
	
	@Id
	@GeneratedValue(strategy=GenerationType.IDENTITY)
	private Long Id;
	
	@NotNull
	private String firstName;
	
	@NotNull
	private String lastName;
	
	@NotNull
	private String login;
	
	@NotNull
	@Column(name="pwd")
	private String password;
	
	@NotNull
	private String email;
	
	@NotNull
	@Column(name="userGroup")
	private String group;

	private int active = 1;
	

	
	public boolean isActive() {
		return active > 0;
	}

	public void setActive(boolean active) {
		if (active)
			this.active = 1;
		else
			this.active = 0;
	}

	public String getGroup() {
		return this.group;
	}
	
	public void setGroup(String group) {
		this.group = group;
	}

	public String getEmail() {
		return email;
	}

	public void setEmail(String email) {
		this.email = email;
	}

	public String getFirstName() {
		return firstName;
	}

	public void setFirstName(String firstName) {
		this.firstName = firstName;
	}

	public String getLastName() {
		return lastName;
	}

	public void setLastName(String lastName) {
		this.lastName = lastName;
	}

	public String getLogin() {
		return login;
	}

	public void setLogin(String login) {
		this.login = login;
	}

	public String getPassword() {
		return password;
	}

	public void setPassword(String password) {
		this.password = password;
	}

	public Long getId() {
		return Id;
	}
	
	public String toString() {
		return "User " + this.login + " id " + this.Id;
	}
	
}
