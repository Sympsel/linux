// public/js/auth.js
document.addEventListener('DOMContentLoaded', function() {
    const loginForm = document.getElementById('loginForm');
    const registerForm = document.getElementById('registerForm');
    const passwordInput = document.getElementById('password');
    const strengthFill = document.getElementById('strengthFill');
    const strengthText = document.getElementById('strengthText');

    if (loginForm) {
        loginForm.addEventListener('submit', function(e) {
            e.preventDefault();
            
            const email = document.getElementById('email').value;
            const password = document.getElementById('password').value;
            
            if (!email || !password) {
                alert('请填写所有字段');
                return;
            }
            
            console.log('Login attempt:', { email });
            alert('登录功能需要后端支持');
        });
    }

    if (registerForm) {
        registerForm.addEventListener('submit', function(e) {
            e.preventDefault();
            
            const username = document.getElementById('username').value;
            const email = document.getElementById('email').value;
            const password = document.getElementById('password').value;
            const confirmPassword = document.getElementById('confirm-password').value;
            const terms = document.getElementById('terms').checked;
            
            if (!username || !email || !password || !confirmPassword) {
                alert('请填写所有字段');
                return;
            }
            
            if (password !== confirmPassword) {
                alert('两次输入的密码不一致');
                return;
            }
            
            if (!terms) {
                alert('请同意服务条款和隐私政策');
                return;
            }
            
            console.log('Register attempt:', { username, email });
            alert('注册功能需要后端支持');
        });

        if (passwordInput && strengthFill) {
            passwordInput.addEventListener('input', function() {
                const password = this.value;
                let strength = 0;
                
                if (password.length >= 8) strength++;
                if (password.length >= 12) strength++;
                if (/[a-z]/.test(password) && /[A-Z]/.test(password)) strength++;
                if (/[0-9]/.test(password)) strength++;
                if (/[^a-zA-Z0-9]/.test(password)) strength++;
                
                strengthFill.className = 'strength-fill';
                
                if (password.length === 0) {
                    strengthText.textContent = '';
                } else if (strength <= 2) {
                    strengthFill.classList.add('weak');
                    strengthText.textContent = '弱';
                    strengthText.style.color = '#ff4444';
                } else if (strength <= 3) {
                    strengthFill.classList.add('medium');
                    strengthText.textContent = '中等';
                    strengthText.style.color = '#ffaa00';
                } else {
                    strengthFill.classList.add('strong');
                    strengthText.textContent = '强';
                    strengthText.style.color = '#00cc66';
                }
            });
        }
    }

    const socialButtons = document.querySelectorAll('.btn-social');
    socialButtons.forEach(btn => {
        btn.addEventListener('click', function() {
            alert('第三方登录功能需要后端配置 OAuth');
        });
    });
});
